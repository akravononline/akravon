#ifndef __INC_ETERLIB_MUTEX_H__
#define __INC_ETERLIB_MUTEX_H__

class Mutex
{
	public:
		Mutex();
		~Mutex();

		void Lock();
		void Unlock();
		bool Trylock();

	private:
		CRITICAL_SECTION lock;
};

#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
