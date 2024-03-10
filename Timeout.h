#pragma once
/*
	2024 Alex Holmes

	This is a timeouts class that uses std::chrono and std::threads to allow you to make functions execute after a delay, or 
	make them repeatedly call after a certain delay between calls, also allowing for only repeating a certain amount of times.
	Made it because I thought the timeout function in javascript was neat and I recently learned how to pass variadiac functions
	and I thought this was a fun way to apply it

	Usage:
	You can use it like a function or like an object, your choice, but if you want to be able to stop the Timeout, you have to 
	use it as an object

	Timeout(2.0f, Func) - Doesn't create an object, just calls a void function with no arguments after a delay
	Timeout(2.0f, Func, "Hello World!") - Doesn't create an object just calls the passed function in 2 seconds
	Timeout(1.0f, Func, true, "Ping!") - Doesn't create an object, loops, calling the passed function every second
	Timeout(1.5f, Func, true, 20, "Ping!") - Doesn't create an object, loops 20 times, calling the function every 1.5 seconds
	
	Timeout tick(0.05f, Update, true, playerCharacter) - Creates an object, calls the passed function 20 times a second, and
														 allows you to stop it or adjust the frequency/delay

	for the second option I would recommend creating them as an object, otherwise, it's a thread that's created in a lambda
	function, there is no way to kill it from outside the constructor, it will just loop infinitely

	I don't like how it's all in an h file, I would much prefer if it was done in a cpp file, but I don't know if I was
	missing something but it refused to let me use templates in a class with the function definitions in the cpp file, so I
	gave up trying and just put them in here, it's especially annoying because I don't like having my includes in my headers
	but I had to otherwise nothing would have worked. This isn't really intended to be made into a library other people use,
	1: Because it has very limited usecases and C++ devs, unlike javascript devs don't include libraries for every simple thing
	2: Because it's really only here because I had an idea and wanted to make it, I recently learned how to use variadiac
	function templates and I figured this would be a fun way to apply them, since application is how I learn.
	I don't mind if it is of course, but the intention is simply because I like making little 1-2 hour projects like this and
	a friend told me I should start throwing them on my github for future use.
	
	I also wanted to get better at using lambdas, which I've known for a very long time but very rarely use, and when I do
	little projects like this, it gives me a resource to look at when I forget, and I feel better about using it because I
	made it
*/

#include <chrono>
#include <thread>

// std::chrono is so long, I almost never use macros but this is gonna be an exception
#define GET_CLOCK std::chrono::system_clock::now().time_since_epoch()
#define GET_TIME(clock) std::chrono::duration_cast<std::chrono::milliseconds>(clock).count()

class Timeout {
public:
	template<typename... Arg>
	Timeout(float DelayS, void (*Func)(Arg...), Arg... Args);
	template<typename... Arg>
	Timeout(float DelayS, bool CallRepeat, void (*Func)(Arg...), Arg... Args);
	template<typename... Arg>
	Timeout(float DelayS, bool CallRepeat, int RepeatCount, void (*Func)(Arg...), Arg... Args);
	void EndTimeout();
	void SetDelay(float DelayS);
private:
	float m_delay = 0.0f;
	bool m_repeat = false;
	bool m_endEarly = false;
};

template<typename... Arg>
Timeout::Timeout(float DelayS, void (*Func)(Arg...), Arg... Args) : m_delay(DelayS) {
	float* pDelay = &m_delay;
	bool* pEndEarly = &m_endEarly;
	std::thread th(
		[Func, Args..., pDelay, pEndEarly]() {
			auto clock1 = GET_CLOCK;
			int time1 = GET_TIME(clock1);
			auto clock2 = GET_CLOCK;
			int time2 = GET_TIME(clock2);

			while (time2 - time1 < *pDelay * 1000.0f) {
				clock2 = GET_CLOCK;
				time2 = GET_TIME(clock2);
				if (*pEndEarly) return;
				Sleep(1); // These are here because otherwise a while loop with this little in it just eats CPU for breakfast
			}

			Func(Args...);
		}
	);

	th.detach();
}

template<typename... Arg>
Timeout::Timeout(float DelayS, bool CallRepeat, void (*Func)(Arg...), Arg... Args) : m_delay(DelayS), m_repeat(CallRepeat) {
	float* pDelay = &m_delay;
	bool* pCallRepeat = &m_repeat;
	bool* pEndEarly = &m_endEarly;
	std::thread th(
		[Func, Args..., pDelay, pCallRepeat, pEndEarly]() {
			do {
				auto clock1 = GET_CLOCK;
				int time1 = GET_TIME(clock1);
				auto clock2 = GET_CLOCK;
				int time2 = GET_TIME(clock2);

				while (time2 - time1 < *pDelay * 1000.0f) {
					clock2 = GET_CLOCK;
					time2 = GET_TIME(clock2);
					if (*pEndEarly) return;
					Sleep(1); // These are here because otherwise a while loop with this little in it just eats CPU for breakfast
				}

				Func(Args...);
			} while (*pCallRepeat);
		}
	);

	th.detach();
}

template<typename... Arg>
Timeout::Timeout(float DelayS, bool CallRepeat, int RepeatCount, void (*Func)(Arg...), Arg... Args) : m_delay(DelayS),
																									  m_repeat(CallRepeat) {
	float* pDelay = &m_delay;
	bool* pCallRepeat = &m_repeat;
	bool* pEndEarly = &m_endEarly;
	std::thread th(
		[Func, Args..., pDelay, pCallRepeat, RepeatCount, pEndEarly]() {
			int reps = RepeatCount;
			do {
				reps--;
				auto clock1 = GET_CLOCK;
				int time1 = GET_TIME(clock1);
				auto clock2 = GET_CLOCK;
				int time2 = GET_TIME(clock2);

				while (time2 - time1 < *pDelay * 1000.0f) {
					clock2 = GET_CLOCK;
					time2 = GET_TIME(clock2);
					if (*pEndEarly) return;
					Sleep(1); // These are here because otherwise a while loop with this little in it just eats CPU for breakfast
				}

				Func(Args...);
			} while (pCallRepeat && reps > 0);
		}
	);

	th.detach();
}

void Timeout::EndTimeout() {
	m_endEarly = true;
}

void Timeout::SetDelay(float DelayS) {
	m_delay = DelayS;
}
