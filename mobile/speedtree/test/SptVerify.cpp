// Cynex: sptverify - cynex_speedtree'yi gercek SpeedTreeRT.dll (x86) ya da dokulmus dll sonuclari (x64) ile karsilastirir
// Cynex: kullanim (x86): sptverify <orijinal.spt> <donusmus.spt> [dokum.bin]
// Cynex: kullanim (x64): sptverify --ref <dokum.bin> <donusmus.spt>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "SptResult.h"

#ifdef SPTVERIFY_WITH_ORACLE
bool RunOracle(CSptResult& r, const unsigned char* pBlock, unsigned int uiSize, bool& bCrashed);
#endif
bool RunCynex(CSptResult& r, const unsigned char* pBlock, unsigned int uiSize);
int FuzzCynex(const unsigned char* pBlock, unsigned int uiSize, unsigned int uiIterations);

static bool ReadFile(const char* pPath, std::vector<unsigned char>& out)
{
	FILE* fp = fopen(pPath, "rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	const long n = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	out.resize(n > 0 ? (size_t) n : 0);
	const bool ok = n > 0 && fread(&out[0], 1, (size_t) n, fp) == (size_t) n;
	fclose(fp);
	return ok;
}

struct SCategory
{
	long long	llRecords = 0;
	long long	llValues = 0;
	long long	llMissing = 0;
	long long	llStructural = 0;
	long long	llExactBad = 0;
	double		dMaxAbs = 0.0;
	double		dMaxRel = 0.0;
	std::string	strWorst;
};

static bool EndsWith(const std::string& s, const char* p)
{
	const size_t n = strlen(p);
	return s.size() >= n && s.compare(s.size() - n, n, p) == 0;
}

static std::string Category(const std::string& key)
{
	std::string c;
	for (size_t i = 0; i < key.size(); ++i)
		if (key[i] < '0' || key[i] > '9')
			c += key[i];
	return c;
}

// Cynex: dll strip birlestirme sirasini her calismada farkli kurabiliyor; ucgen kumesi (yon korunarak) ayni mi
static bool SameTriangleSet(const std::vector<double>& a, const std::vector<double>& b)
{
	std::vector<std::vector<int> > ta, tb;
	const std::vector<double>* src[2] = { &a, &b };
	std::vector<std::vector<int> >* dst[2] = { &ta, &tb };
	for (int s = 0; s < 2; ++s)
	{
		const std::vector<double>& v = *src[s];
		for (size_t i = 2; i < v.size(); ++i)
		{
			int t[3] = { (int) v[i - 2], (int) v[i - 1], (int) v[i] };
			if (t[0] == t[1] || t[1] == t[2] || t[0] == t[2])
				continue;
			if (i & 1)
				std::swap(t[0], t[1]);
			// Cynex: en kucuk indeks basa (donus yonu korunur)
			while (t[0] > t[1] || t[0] > t[2])
			{
				const int x = t[0];
				t[0] = t[1]; t[1] = t[2]; t[2] = x;
			}
			dst[s]->push_back(std::vector<int>(t, t + 3));
		}
		std::sort(dst[s]->begin(), dst[s]->end());
	}
	return ta == tb;
}

static int Compare(const CSptResult& a, const CSptResult& b, int iVerbose)
{
	std::map<std::string, std::vector<const SSptRecord*> > kB;
	for (size_t i = 0; i < b.m_vecRecords.size(); ++i)
		kB[b.m_vecRecords[i].m_strKey].push_back(&b.m_vecRecords[i]);
	std::map<std::string, size_t> kUsed;
	std::map<std::string, SCategory> kCats;
	int iPrinted = 0;
	long long llExtra = 0;

	for (size_t i = 0; i < a.m_vecRecords.size(); ++i)
	{
		const SSptRecord& ra = a.m_vecRecords[i];
		SCategory& c = kCats[Category(ra.m_strKey)];
		++c.llRecords;
		const size_t occ = kUsed[ra.m_strKey]++;
		std::map<std::string, std::vector<const SSptRecord*> >::const_iterator it = kB.find(ra.m_strKey);
		if (it == kB.end() || occ >= it->second.size())
		{
			++c.llMissing;
			if (iVerbose && iPrinted++ < 40)
				printf("  MISSING %s\n", ra.m_strKey.c_str());
			continue;
		}
		const SSptRecord& rb = *it->second[occ];
		if (ra.m_strKey.size() > 6 && ra.m_strKey.compare(ra.m_strKey.size() - 6, 6, ":strip") == 0 && ra.m_vecValues != rb.m_vecValues)
		{
			// Cynex: sira farkli olabilir; ucgen kumesi ayniysa gecerli say
			c.llValues += (long long) ra.m_vecValues.size();
			if (!SameTriangleSet(ra.m_vecValues, rb.m_vecValues))
			{
				++c.llExactBad;
				if (iVerbose && iPrinted++ < 40)
					printf("  STRIPSET %s\n", ra.m_strKey.c_str());
			}
			else
				++kCats[Category(ra.m_strKey) + "(reordered)"].llRecords;
			continue;
		}
		// Cynex: dll'de rastgele (CRT rand durumu) olan ruzgar matris indeksleri ve strip uzunluklari yumusak karsilastirma
		const bool bSoft = EndsWith(ra.m_strKey, ":windi") || EndsWith(ra.m_strKey, ":striplen");
		if (bSoft && ra.m_vecValues != rb.m_vecValues)
		{
			++kCats[Category(ra.m_strKey) + "(soft)"].llExactBad;
			continue;
		}
		if (ra.m_iKind != rb.m_iKind || ra.m_vecValues.size() != rb.m_vecValues.size())
		{
			++c.llStructural;
			if (iVerbose && iPrinted++ < 40)
				printf("  STRUCT %s (len %zu vs %zu)\n", ra.m_strKey.c_str(), ra.m_vecValues.size(), rb.m_vecValues.size());
			continue;
		}
		if (ra.m_iKind == CSptResult::KIND_STRING)
		{
			++c.llValues;
			if (ra.m_strText != rb.m_strText)
			{
				++c.llExactBad;
				if (iVerbose && iPrinted++ < 40)
					printf("  STRING %s '%s' vs '%s'\n", ra.m_strKey.c_str(), ra.m_strText.c_str(), rb.m_strText.c_str());
			}
			continue;
		}
		for (size_t k = 0; k < ra.m_vecValues.size(); ++k)
		{
			const double x = ra.m_vecValues[k], y = rb.m_vecValues[k];
			++c.llValues;
			if (ra.m_iKind == CSptResult::KIND_FLOAT)
			{
				const bool bNanA = x != x, bNanB = y != y;
				if (bNanA || bNanB)
				{
					if (bNanA != bNanB)
						++c.llExactBad;
					continue;
				}
				const double d = fabs(x - y);
				const double rel = d / (fabs(x) > 1.0 ? fabs(x) : 1.0);
				if (rel > c.dMaxRel)
				{
					c.dMaxRel = rel;
					char sz[256];
					snprintf(sz, sizeof(sz), "%s[%zu] %.9g vs %.9g", ra.m_strKey.c_str(), k, x, y);
					c.strWorst = sz;
				}
				if (d > c.dMaxAbs)
					c.dMaxAbs = d;
			}
			else if (x != y)
			{
				++c.llExactBad;
				if (iVerbose && iPrinted++ < 40)
					printf("  VALUE %s[%zu] %.9g vs %.9g\n", ra.m_strKey.c_str(), k, x, y);
			}
		}
	}
	for (std::map<std::string, std::vector<const SSptRecord*> >::const_iterator it = kB.begin(); it != kB.end(); ++it)
	{
		const size_t used = kUsed.count(it->first) ? kUsed[it->first] : 0;
		if (it->second.size() > used)
		{
			llExtra += (long long) (it->second.size() - used);
			if (iVerbose && iPrinted++ < 40)
				printf("  EXTRA %s\n", it->first.c_str());
		}
	}

	long long llBad = llExtra;
	for (std::map<std::string, SCategory>::const_iterator it = kCats.begin(); it != kCats.end(); ++it)
	{
		const SCategory& c = it->second;
		if (!EndsWith(it->first, "(soft)"))
			llBad += c.llMissing + c.llStructural + c.llExactBad;
		printf("CAT %-34s rec=%lld val=%lld miss=%lld struct=%lld bad=%lld maxabs=%.3g maxrel=%.3g%s%s\n", it->first.c_str(), c.llRecords, c.llValues,
			c.llMissing, c.llStructural, c.llExactBad, c.dMaxAbs, c.dMaxRel, c.strWorst.empty() ? "" : " worst=", c.strWorst.c_str());
	}
	printf("EXTRA %lld\n", llExtra);
	printf("TOTAL records=%zu badcount=%lld\n", a.m_vecRecords.size(), llBad);
	return llBad == 0 ? 0 : 1;
}

int main(int argc, char** argv)
{
	int iVerbose = getenv("SPTVERIFY_VERBOSE") ? 1 : 0;
	if (argc >= 3 && strcmp(argv[1], "--fuzz") == 0)
	{
		std::vector<unsigned char> conv;
		if (!ReadFile(argv[2], conv))
			return 3;
		printf("FUZZ loaded=%d\n", FuzzCynex(&conv[0], (unsigned int) conv.size(), argc >= 4 ? (unsigned int) atoi(argv[3]) : 500));
		return 0;
	}
	if (argc >= 4 && strcmp(argv[1], "--ref") == 0)
	{
		CSptResult ref, mine;
		std::vector<unsigned char> conv;
		if (!ref.Load(argv[2]) || !ReadFile(argv[3], conv))
		{
			printf("ERROR okunamadi\n");
			return 3;
		}
		RunCynex(mine, &conv[0], (unsigned int) conv.size());
		return Compare(ref, mine, iVerbose);
	}
#ifdef SPTVERIFY_WITH_ORACLE
	if (argc >= 3)
	{
		std::vector<unsigned char> orig, conv;
		if (!ReadFile(argv[1], orig) || !ReadFile(argv[2], conv))
		{
			printf("ERROR okunamadi\n");
			return 3;
		}
		CSptResult ra, rb;
		bool bCrashed = false;
		const bool bOk = RunOracle(ra, &orig[0], (unsigned int) orig.size(), bCrashed);
		if (bCrashed)
		{
			printf("ORACLECRASH records=%zu\n", ra.m_vecRecords.size());
			return 4;
		}
		if (argc >= 4)
			ra.Save(argv[3]);
		RunCynex(rb, &conv[0], (unsigned int) conv.size());
		(void) bOk;
		const int ret = Compare(ra, rb, iVerbose);
		fflush(stdout);
		return ret;
	}
#endif
	printf("kullanim: sptverify <orijinal.spt> <donusmus.spt> [dokum.bin] | --ref <dokum.bin> <donusmus.spt>\n");
	return 1;
}
