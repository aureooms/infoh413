#include <iostream>
#include <fstream>

#include "lib/io.hpp"
#include "lib/error/exception.hpp"

#include "pfsp/io/parse/body.hpp"
#include "pfsp/io/parse/header.hpp"
#include "pfsp/mem/allocate.hpp"

#include "pfsp_vnd/global.hpp"
#include "pfsp_vnd/config.hpp"

using namespace pfsp_vnd;


void run(){

// INPUT
	
	std::ifstream fileIn;
	fileIn.open(global::params[0]);

	if(!fileIn.is_open()) throw lib::error::exception("Could not open input file");

	pfsp::io::parse::header(fileIn, global::i.nbJob, global::i.nbMac);
	pfsp::mem::allocate(global::i.processing, global::i.dueDates, global::i.priority, global::i.nbJob, global::i.nbMac);
	global::proxy.resize(&global::i.processing[0], global::i.nbJob + 1, global::i.nbMac + 1);
	pfsp::io::parse::body(fileIn, global::i.nbJob, global::i.nbMac, global::proxy, global::i.dueDates, global::i.priority);

	fileIn.close();

	if(global::verbose){
		std::cout << "Number of jobs : " << global::i.nbJob << std::endl;
		std::cout << "Number of machines : " << global::i.nbMac << std::endl;
	}

	/* evaluator functor */
	E e(global::i.nbJob, global::i.nbMac, global::i.dueDates, global::i.priority, global::proxy);
	TE te(global::i.nbJob, global::i.nbMac, global::i.dueDates, global::i.priority, global::proxy, e.wt, e.detail);
	IE ie(global::i.nbJob, global::i.nbMac, global::i.dueDates, global::i.priority, global::proxy, e.wt, e.detail);
	EE ee(global::i.nbJob, global::i.nbMac, global::i.dueDates, global::i.priority, global::proxy, e.wt, e.detail);
	global::transpose.eval = &te;
	global::insert.eval = &ie; 
	global::exchange.eval = &ee;

	/* 1 based index FTW !¸··}#{}¼CHANGE THIS FUCKSHIT */
	S s(e.nbJob + 1);

// GEN INITIAL SOLUTION

	auto init = global::init[global::options["--init"][0]];
	(*init)(s);
	val_t opt = e(s);

	if(global::verbose){
		std::cout << "init: ";
		lib::io::format(std::cout, s, global::list_p) << std::endl;
		std::cout << opt << std::endl;
	}

	auto ordering = global::ordering[global::options["--ordering"][0]];
	auto pivoting = global::pivoting[global::options["--pivoting"][0]];

// FIND LOCAL OPTIMUM

	hrclock::time_point beg = hrclock::now();

	size_t k = 0;

	while(k < ordering.size()){
		R best = pivoting(s, ordering[k]->walk, ordering[k]->eval);
		while(best.first){
			opt += best.first;
			(*ordering[k]->eval)(s, best.second, e.detail, e.wt);
			(*ordering[k]->apply)(s, best.second);
			k = 0;
			best = pivoting(s, ordering[k]->walk, ordering[k]->eval);
		}
		++k;
	}
	
	hrclock::time_point end  = hrclock::now();

// OUTPUT

	// LO

	if(global::verbose){
		std::cout << "best: ";
		lib::io::format(std::cout, s, global::list_p) << std::endl;
	}

	std::cout << opt << std::endl;

	// TIME

	if(global::verbose) std::cout << "time : ";
	std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);
	std::cout << duration.count();
	if(global::verbose) std::cout << " ms";
	std::cout << std::endl;
	
	// SEED

	if(global::verbose) std::cout << "seed : ";
	lib::io::format(std::cout, global::seed_v, global::list_p) << std::endl;
}


int main(int argc, char *argv[]){
	
	config::fill(argc, argv);

	if(global::help){
		config::help();
		return 0;
	}

	try{
		config::check();
		run();
	}
	catch(const std::exception& e){
		std::cout << "error -> " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
