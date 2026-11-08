///////////////////////////////////////////////////////////////////////
//	SptResult.h
//
//	Cynex: anahtarli sonuc kaydi (dll / cynex karsilastirmasi, x64 icin dosyaya dokum).

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

struct SSptRecord
{
	std::string			m_strKey;
	int					m_iKind;		// 0 float, 1 int, 2 string, 3 ptr, 4 color
	std::vector<double>	m_vecValues;
	std::string			m_strText;
};

class CSptResult
{
	public:
		enum { KIND_FLOAT, KIND_INT, KIND_STRING, KIND_PTR, KIND_COLOR };

		std::vector<SSptRecord>	m_vecRecords;

		SSptRecord& Add(const char* a, const char* b, int iKind)
		{
			m_vecRecords.push_back(SSptRecord());
			SSptRecord& r = m_vecRecords.back();
			r.m_strKey = std::string(a) + ":" + b;
			r.m_iKind = iKind;
			return r;
		}
		void Float(const char* a, const char* b, float v)	{ Add(a, b, KIND_FLOAT).m_vecValues.push_back(v); }
		void Int(const char* a, const char* b, long long v)	{ Add(a, b, KIND_INT).m_vecValues.push_back((double) v); }
		void Ptr(const char* a, const char* b, const void* p)	{ Add(a, b, KIND_PTR).m_vecValues.push_back(p ? 1.0 : 0.0); }
		void String(const char* a, const char* b, const char* s)
		{
			SSptRecord& r = Add(a, b, KIND_STRING);
			r.m_strText = s ? s : "(null)";
		}
		void Floats(const char* a, const char* b, const float* p, size_t n)
		{
			SSptRecord& r = Add(a, b, KIND_FLOAT);
			r.m_vecValues.resize(n);
			for (size_t i = 0; i < n; ++i)
				r.m_vecValues[i] = p[i];
		}
		void Colors(const char* a, const char* b, const unsigned long* p, size_t n)
		{
			SSptRecord& r = Add(a, b, KIND_COLOR);
			r.m_vecValues.resize(n);
			for (size_t i = 0; i < n; ++i)
				r.m_vecValues[i] = (double) (uint32_t) p[i];
		}
		void U8s(const char* a, const char* b, const unsigned char* p, size_t n)
		{
			SSptRecord& r = Add(a, b, KIND_INT);
			r.m_vecValues.resize(n);
			for (size_t i = 0; i < n; ++i)
				r.m_vecValues[i] = p[i];
		}
		void U16s(const char* a, const char* b, const unsigned short* p, size_t n)
		{
			SSptRecord& r = Add(a, b, KIND_INT);
			r.m_vecValues.resize(n);
			for (size_t i = 0; i < n; ++i)
				r.m_vecValues[i] = p[i];
		}

		bool Save(const char* pPath) const
		{
			FILE* fp = fopen(pPath, "wb");
			if (!fp)
				return false;
			const uint32_t n = (uint32_t) m_vecRecords.size();
			fwrite(&n, 4, 1, fp);
			for (size_t i = 0; i < m_vecRecords.size(); ++i)
			{
				const SSptRecord& r = m_vecRecords[i];
				const uint32_t k = (uint32_t) r.m_strKey.size(), t = (uint32_t) r.m_strText.size(), c = (uint32_t) r.m_vecValues.size();
				const int32_t kind = r.m_iKind;
				fwrite(&k, 4, 1, fp); fwrite(r.m_strKey.data(), 1, k, fp);
				fwrite(&kind, 4, 1, fp);
				fwrite(&t, 4, 1, fp); fwrite(r.m_strText.data(), 1, t, fp);
				fwrite(&c, 4, 1, fp);
				if (c)
					fwrite(&r.m_vecValues[0], 8, c, fp);
			}
			fclose(fp);
			return true;
		}
		bool Load(const char* pPath)
		{
			FILE* fp = fopen(pPath, "rb");
			if (!fp)
				return false;
			uint32_t n = 0;
			if (fread(&n, 4, 1, fp) != 1)
			{
				fclose(fp);
				return false;
			}
			m_vecRecords.resize(n);
			bool bOk = true;
			for (uint32_t i = 0; i < n && bOk; ++i)
			{
				SSptRecord& r = m_vecRecords[i];
				uint32_t k = 0, t = 0, c = 0;
				int32_t kind = 0;
				bOk = fread(&k, 4, 1, fp) == 1;
				r.m_strKey.resize(k);
				if (k) bOk = bOk && fread(&r.m_strKey[0], 1, k, fp) == k;
				bOk = bOk && fread(&kind, 4, 1, fp) == 1;
				r.m_iKind = kind;
				bOk = bOk && fread(&t, 4, 1, fp) == 1;
				r.m_strText.resize(t);
				if (t) bOk = bOk && fread(&r.m_strText[0], 1, t, fp) == t;
				bOk = bOk && fread(&c, 4, 1, fp) == 1;
				r.m_vecValues.resize(c);
				if (c) bOk = bOk && fread(&r.m_vecValues[0], 8, c, fp) == c;
			}
			fclose(fp);
			return bOk;
		}
};
