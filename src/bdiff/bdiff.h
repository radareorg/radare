/*
 * Copyright (C) 2008 Daniel Fernandez <soyfeliz@48bits.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <utility>
#ifndef H_BDIFF
#define H_BDIFF
#define W_BIT (sizeof(ulong)*CHAR_BIT)

class BDiff
{
	typedef const unsigned char *Mindex ;
	class BitVec ;
	class SubSeq
	{
		friend class BDiff ;
		struct Elem
		{
			size_t i, j, n ;
			Elem() : i(0), j(0), n(0) {}
			Elem(size_t a, size_t b, size_t c) : i(a), j(b), n(c) {}
		} ;
		std::list<Elem> elem ;
		size_t elems ;
	public:
		SubSeq() : elems(0) {}
		SubSeq *operator+=(const std::pair<size_t, size_t> &r)
		{
			if (elem.empty() || elem.back().i + elem.back().n != r.first
			|| elem.back().j + elem.back().n != r.second)
				elem.push_back(Elem(r.first, r.second, 1)) ;
			else
				++(elem.back().n) ;

			++elems ;
			return this ;
		}
	} ;
	void gen_diff() ;
	void FindRow(const char *, const char *, size_t, size_t, size_t *) ;
	void FindRowR(const char *,const char *, size_t, size_t, size_t *) ;
	void LCS(const char *, const char *, size_t, size_t) ;
	const char *X ;
	const char *Y ;
	SubSeq C ;
public:
	enum Action { INS, REM, SUB } ;
	struct Edit
	{
		Action act ;
		size_t offset, sz ;
		std::vector<char> buff ;
		Edit(Action a, size_t b, size_t c, const char *d = NULL, const char *e = NULL) :
		act(a), offset(b), sz(c)
		{
			if (d && e)
				buff = std::vector<char>(d, e) ;
		}
	} ;

	BDiff(const char *, size_t , const char *, size_t) ;
	std::list<Edit> diff ;
} ;

class BDiff::BitVec
{
	size_t *use, bsz, size ;
	ulong *bits ;
public:
	BitVec(size_t b) :
		bsz(b), size(b/W_BIT+1), bits(new ulong [size]), use(new size_t(1)) {}

	BitVec(size_t b, size_t c) :
		bsz(b), size(b/W_BIT+1), bits(new ulong [size]), use(new size_t(1))
	{
		size_t x ;

		for(x = 0; x < c/W_BIT; ++x)
			bits[x] = ~0UL ;

		bits[x] = (1UL << c%W_BIT) - 1 ;

		while (++x < size)
			bits[x] = 0 ;
	}

	BitVec(const BitVec &i) :
		bits(i.bits), bsz(i.bsz), size(i.size), use(i.use) { ++*use ; }

	~BitVec()
	{
		if (--*use == 0)
		{
			delete [] bits ;
			delete use ;
		}
	}

	BitVec &operator=(const BitVec &r)
	{
		++*r.use ;

		if (--*use == 0)
		{
			delete [] bits ;
			delete use ;
		}

		bits = r.bits ;
		use = r.use ;
		return *this ;
	}

	BitVec operator+(const BitVec &r)
	{
		BitVec sum(bsz) ;
		int cf(0) ;

		for (size_t x = 0 ; x < size; ++x)
		{
			sum.bits[x] = bits[x] + r.bits[x] + cf ;
			cf = (~0UL - bits[x] < r.bits[x]) ? 1 : 0 ;
		}

		sum.bits[0] += cf + (sum.bits[bsz/W_BIT] >> (bsz+1)%W_BIT) ;
		return sum ;
	}

	BitVec operator&(const BitVec &r)
	{
		BitVec ret(bsz) ;

		for (size_t x = 0 ; x < size; ++x)
			ret.bits[x] = bits[x] & r.bits[x] ;

		return ret ;
	}

	BitVec operator|(const BitVec &r)
	{
		BitVec ret(bsz) ;

		for (size_t x = 0 ; x < size; ++x)
			ret.bits[x] = bits[x] | r.bits[x] ;

		return ret ;
	}

	BitVec operator~()
	{
		BitVec ret(bsz) ;

		for (size_t x = 0 ; x < size; ++x)
			ret.bits[x] = ~bits[x] ;

		ret.bits[bsz/W_BIT] &= (1UL << bsz%W_BIT) - 1 ;
		return ret ;
	}

	void set(size_t i)
	{
		bits[i/W_BIT] |= (1UL << i%W_BIT) ;
	}

	bool carry()
	{
		if (bits[bsz/W_BIT] & (1UL << bsz%W_BIT))
		{
			bits[bsz/W_BIT] &= (1UL << bsz%W_BIT) - 1 ;
			return true ;
		}

		return false ;
	}
} ;

BDiff::BDiff(const char *a, size_t m, const char *b, size_t n) : X(a), Y(b)
{
	size_t head, tail, orig_m(m), orig_n(n) ;

	for (head = 0; head < std::min(m, n) && X[head] == Y[head]; ++head) ;

	X = &X[head] ;
	Y = &Y[head] ;
	m -= head ;
	n -= head ;

	for (tail = 0; m && n && X[m-1] == Y[n-1]; ++tail, --m, --n) ;

	LCS(X, Y, m, n) ;
			
	if (head)
	{
		// correjimos los offsets
		for (std::list<SubSeq::Elem>::iterator b = C.elem.begin(); b != C.elem.end(); ++b)
		{
			(*b).i += head ;
			(*b).j += head ;
		}

		// y le insertamos la cabeza >;)
		C.elem.push_front(SubSeq::Elem(0, 0, head)) ;
	}

	if (tail)
		C.elem.push_back(SubSeq::Elem(m+head, n+head, tail)) ;
	else
		C.elem.push_back(SubSeq::Elem(orig_m, orig_n, 0)) ;

	X = a ;
	Y = b ;
	gen_diff() ;
	C = SubSeq() ; // memoria inutil
}

void BDiff::FindRow(const char *x,const char *y, size_t m, size_t n, size_t *L)
{
	std::vector<BitVec> M (1<<CHAR_BIT,BitVec(m, 0));
	BitVec S(m, m) ;

	for(size_t i = 0; i < m; ++i)
		M[((Mindex)x)[i]].set(i) ;

	L[0] = 0 ;

	for(size_t j = 1; j <= n; ++j)
	{
		S = (S + (S & M[((Mindex)y)[j-1]])) | (S & ~M[((Mindex)y)[j-1]]) ;

		if (S.carry())
			L[j] = L[j-1] + 1 ;
		else
			L[j] = L[j-1] ;
	}

	//delete [] M ;
}

/*
	Para no tener que crear los arreglos invertidos de x e y
	mejor hacemos una version de FindRow que lea los elementos
	en sentido contrario
*/
void BDiff::FindRowR(const char *x, const char *y, size_t m, size_t n, size_t *L)
{
	std::vector<BitVec> M (1<<CHAR_BIT,BitVec(m, 0));
	BitVec S(m, m) ;

	for(size_t i = 0; i < m; ++i)
		M[((Mindex)x)[m-i-1]].set(i) ;

	L[0] = 0 ;

	for(size_t j = 1; j <= n; ++j)
	{
		S = (S + (S & M[((Mindex)y)[n-j]])) | (S & ~M[((Mindex)y)[n-j]]) ;

		if (S.carry())
			L[j] = L[j-1] + 1 ;
		else
			L[j] = L[j-1] ;
	}

	//delete M ;
}

void BDiff::LCS(const char *x, const char *y, size_t m, size_t n)
{
	if (!n)
		return ;

	if (m == 1)
	{
		for(size_t j = 0; j <= n; ++j)
			if (x[0] == y[j])
			{
				C += std::make_pair(x - X, &y[j] - Y) ;
				break ;
			}

		return ;
	}

	size_t i(m/2), k(0), *L(new size_t [n+1]), *Lr(new size_t [n+1]) ;

	FindRow(x, y, i, n, L) ;
	FindRowR(&x[i], y, m-i, n, Lr) ;

	for (size_t max = 0, l = 0; l <= n; ++l)
		if (L[l] + Lr[n-l] > max)
		{
			max = L[l] + Lr[n-l] ;
			k = l ;
		}

	delete [] L ;
	delete [] Lr ;

	LCS(x, y, i, k) ;
	LCS(&x[i], &y[k], m-i, n-k) ;
}

void BDiff::gen_diff()
{
	size_t x(0), y(0), delta(0), tmp(0) ;

	for (std::list<SubSeq::Elem>::iterator b = C.elem.begin(); b != C.elem.end(); ++b)
	{
		if ((*b).i > x && (*b).j > y)
		{
			size_t n((*b).i-x), m((*b).j-y) ;

			if (n == m)
				diff.push_back(Edit(SUB, x+delta, (*b).j-y, &Y[y], &Y[(*b).j])) ;

			if (n < m)
			{
				diff.push_back(Edit(SUB, x+delta, n, &Y[y], &Y[y+n])) ;
				diff.push_back(Edit(INS, x+delta+n, (*b).j-y-n, &Y[y+n], &Y[(*b).j])) ;
				tmp += (*b).j-y-n ;
			}

			if (n > m)
			{
				diff.push_back(Edit(SUB, x+delta, m, &Y[y], &Y[(*b).j])) ;
				diff.push_back(Edit(REM, x+delta+m, (*b).i-x-m)) ;
				tmp -= (*b).i-x-m ;
			}
		}
		else
		{
			if ((*b).i > x)
			{
				diff.push_back(Edit(REM, x+delta, (*b).i-x)) ;
				tmp -= (*b).i-x ;
			}

			if ((*b).j > y)
			{
				diff.push_back(Edit(INS, x+delta, (*b).j-y, &Y[y], &Y[(*b).j])) ;
				tmp += (*b).j-y ;
			}
		}

		delta = tmp ;
		x = (*b).i + (*b).n ;
		y = (*b).j + (*b).n ;
	}
}

#endif
