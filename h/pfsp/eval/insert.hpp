#ifndef _PFSP_EVAL_INSERT_HPP
#define _PFSP_EVAL_INSERT_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <tuple>

#include "pfsp/eval/functor.hpp"

namespace pfsp{
namespace eval{

template<
	typename addr_t,
	typename val_t,
	typename priority_t,
	typename S,
	typename M,
	typename A1,
	typename A2,
	typename A3,
	typename A4,
	typename A5
>
class insert : public functor<val_t, S, M>{
public:

	const addr_t& nbJob;
	const addr_t& nbMac;

	const A1& dueDates;
	const A2& priority;
	const A3& processing;

	A4 detail;
	A5 wt;
	const A5& wt_r;
	const A4& detail_r;

	insert(const addr_t& nbJob, const addr_t& nbMac, const A1& dueDates,
		const A2& priority, const A3& processing, const A5& wt_r, const A4& detail_r)
	:nbJob(nbJob), nbMac(nbMac), dueDates(dueDates),
	priority(priority), processing(processing),
	detail(nbJob + 1), wt(nbJob + 1, 0), wt_r(wt_r), detail_r(detail_r){
		for(addr_t i = 0; i <= nbJob; ++i) detail[i].resize(nbMac + 1, 0);
	}

	virtual val_t operator()(const S& sol, const M& mutation){
		addr_t beg, end, x, l, r, t;
		std::tie(beg, end) = mutation;


		if(beg < end){
			x = 1;
			l = beg;
			r = end - 1;
			t = r + 2;

			// FETCH PREVIOUS STATE
			for(addr_t i = 0; i <= nbMac; ++i) detail[beg-1][i] = detail_r[beg-1][i];
		}
		else{
			x = -1;
			l = end + 1;
			r = beg;
			t = r + 1;

			// FETCH PREVIOUS STATE
			for(addr_t i = 0; i <= nbMac; ++i) detail[end-1][i] = detail_r[end-1][i];
				
			detail[end][1] = detail[end-1][1] + processing[sol[beg]][1];
			for(addr_t m = 2; m <= nbMac; ++m){
				detail[end][m] = std::max(detail[end][m-1], detail[end-1][m]) + processing[sol[beg]][m];
			}
		}

		for(addr_t j = l; j <= r; ++j) detail[j][1] = detail[j-1][1] + processing[sol[j+x]][1];
		for(addr_t m = 2; m <= nbMac; ++m){
			for(addr_t j = l; j <= r; ++j){
				detail[j][m] = std::max(detail[j][m-1], detail[j-1][m]) + processing[sol[j+x]][m];
			}
		}

		if(x == 1){
			detail[end][1] = detail[end-1][1] + processing[sol[beg]][1];
			for(addr_t m = 2; m <= nbMac; ++m){
				detail[end][m] = std::max(detail[end][m-1], detail[end-1][m]) + processing[sol[beg]][m];
			}
		}

		for(addr_t j = t; j <= nbJob; ++j) detail[j][1] = detail[j-1][1] + processing[sol[j]][1];
		for(addr_t m = 2; m <= nbMac; ++m){
			for(addr_t j = t; j <= nbJob; ++j){
				detail[j][m] = std::max(detail[j][m-1], detail[j-1][m]) + processing[sol[j]][m];
			}
		}


		val_t wtd = 0;
		for(addr_t j = l; j <= r; ++j){
			wt[j] = (std::max(detail[j][nbMac] - dueDates[sol[j+x]], 0L) * priority[sol[j+x]]); 
			wtd += wt[j] - wt_r[j];
		}
		for(addr_t j = t; j <= nbJob; ++j){
			wt[j] = (std::max(detail[j][nbMac] - dueDates[sol[j]], 0L) * priority[sol[j]]); 
			wtd += wt[j] - wt_r[j];
		}

		wt[end] = (std::max(detail[end][nbMac] - dueDates[sol[beg]], 0L) * priority[sol[beg]]);

		return wtd + wt[end] - wt_r[end];
	}
};

}
}



#endif // _PFSP_EVAL_INSERT_HPP
