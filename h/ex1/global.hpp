#ifndef _EX1_GLOBAL_HPP
#define _EX1_GLOBAL_HPP


#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <chrono>
#include <random>

#include "pfsp/neighborhood/exchange.hpp"
#include "pfsp/neighborhood/insert.hpp"
#include "pfsp/neighborhood/transpose.hpp"

#include "pfsp/pivoting/functor.hpp"
#include "pfsp/pivoting/best.hpp"
#include "pfsp/pivoting/first.hpp"

#include "pfsp/init/functor.hpp"
#include "pfsp/init/random.hpp"
#include "pfsp/init/slack.hpp"


typedef int addr_t;
typedef long int val_t;
typedef long int priority_t;

typedef std::vector<addr_t> solution;

typedef std::default_random_engine random_engine;
typedef std::uniform_int_distribution<size_t> uniform_distribution;

typedef std::chrono::system_clock sysclock;
typedef std::chrono::high_resolution_clock hrclock;

typedef pfsp::pivoting::functor<solution>* handler;
typedef void (*walk)(const solution&, handler);
typedef pfsp::eval<addr_t, val_t, priority_t> eval;

typedef solution (*P)(const solution&, walk, eval);

namespace ex1{
	namespace global{


	// DATA
		
		const char* list_p[] = {"", "", " "};

	// CHRONO

		double start, stop, start_c, stop_c;
		hrclock::time_point checkpoint, tmp;
		hrclock::duration duration(0);


	// SEED

		std::vector<long long> seed_v;
		random_engine g;


	// FLAGS

		bool help;

	// OPTIONS

		auto init_r = pfsp::init::random<random_engine, uniform_distribution, solution, size_t>(g);

		std::unordered_map<std::string, pfsp::init::functor<solution>*> init{
			{"random", &init_r.f},
			{"slack", nullptr} // TODO
		};

		std::unordered_map<std::string, walk> neighborhood{
			{"exchange" , &pfsp::neighborhood::exchange<solution, handler>},
			{"insert" , &pfsp::neighborhood::insert<solution, handler>},
			{"transpose" , &pfsp::neighborhood::transpose<solution, handler>}
		};

		std::unordered_map<std::string, P> pivoting{
			{"best", &pfsp::pivoting::best<solution, walk, eval>},
			{"first", &pfsp::pivoting::first<solution, walk, eval>}
		};


	// INPUT

		std::vector<std::string> params;
		std::map<std::string, std::vector<std::string>> options;
		std::set<std::string> flags;
		std::set<std::string> option_set = {
			"--seed",
			"--init",
			"--pivoting",
			"--neighborhood",
		};
		std::set<std::string> flag_set = {
			"-h", "--help"
		};

	}
}



#endif // _EX1_GLOBAL_HPP