#pragma once

class CReferenceObject
{
	public:
		CReferenceObject();
		virtual ~CReferenceObject();

		void AddReference();
		void AddReferenceOnly();
		void Release();

		int GetReferenceCount();

		bool canDestroy();

	protected:
		virtual void OnConstruct();
		virtual void OnSelfDestruct();

	private:
		int m_refCount;
		bool m_destructed;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
