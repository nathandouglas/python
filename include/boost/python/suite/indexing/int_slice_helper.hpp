// -*- mode:c++ -*-
//
// Header file int_slice_helper.hpp
//
// Copyright (c) 2003 Raoul M. Gough
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)
//
// History
// =======
// 2003/10/13   rmg     File creation
//
// $Id$
//

#ifndef int_slice_helper_rmg_20031013_included
#define int_slice_helper_rmg_20031013_included

#include <boost/python/suite/indexing/slice.hpp>
#include <boost/python/errors.hpp>

namespace boost { namespace python { namespace indexing {
  template<typename Algorithms>
  struct int_slice_helper
  {
    typedef Algorithms algorithms;
    typedef typename algorithms::container container;
    typedef typename algorithms::reference reference;
    typedef typename algorithms::value_param value_param;
    typedef typename algorithms::container_traits container_traits;

    int_slice_helper (slice const &sl, container &c);

    bool next();

    reference current () const;
    void write (value_param val);
    void erase_remaining () const;

  private:
    void assign (value_param val) const;
    void insert (value_param val);

  private:
    integer_slice mSlice;
    container *mPtr;
    int mPos;
  };

  template<typename Algorithms>
  int_slice_helper<Algorithms>::int_slice_helper (slice const &sl
                                                  , container &c)
    : mSlice (sl, algorithms::size (c))
    , mPtr (&c)
    , mPos (-1)
  {
  }

  template<typename Algorithms>
  bool
  int_slice_helper<Algorithms>::next()
  {
    bool result = false; // Assume the worst

    if (mPos == -1)
      {
        // First time call - get to start of the slice (if any)
        mPos = mSlice.start();
        result = mSlice.in_range (mPos);
      }

    else if (mSlice.in_range (mPos))
      {
        // Subsequent calls - advance by the slice's stride
        mPos += mSlice.step();
        result = mSlice.in_range (mPos);
      }

    return result;
  }

  template<typename Algorithms>
  typename int_slice_helper<Algorithms>::reference
  int_slice_helper<Algorithms>::current () const
  {
    return algorithms::get (*mPtr, mPos);
  }

  template<typename Algorithms>
  void int_slice_helper<Algorithms>::write (value_param val)
  {
    if (next())
      {
        assign (val);
      }

    else
      {
        insert (val);
      }
  }

  template<typename Algorithms>
  void int_slice_helper<Algorithms>::assign (value_param val) const
  {
    algorithms::assign (*mPtr, mPos, val);
  }

  namespace detail {
    template<bool doit> struct maybe_insert {
      template<class Algorithms>
      static void apply (typename Algorithms::container &
                         , typename Algorithms::index_param
                         , typename Algorithms::value_param)
      {
        PyErr_SetString (PyExc_TypeError
                         , "container does not support insertion into slice");

        boost::python::throw_error_already_set ();
      }
    };

    template<> struct maybe_insert<true> {
      template<class Algorithms>
      static void apply (typename Algorithms::container &c
                         , typename Algorithms::index_param i
                         , typename Algorithms::value_param v)
      {
        Algorithms::insert (c, i, v);
      }
    };
  }

  template<typename Algorithms>
  void int_slice_helper<Algorithms>::insert (value_param val)
  {
    if (mSlice.step() != 1)
      {
        PyErr_SetString (PyExc_ValueError
                         , "attempt to insert via extended slice");

        boost::python::throw_error_already_set ();
      }

    else
      {
        detail::maybe_insert<container_traits::has_insert>
          ::template apply<Algorithms> (*mPtr, mPos, val);

        ++mPos;  // Advance for any subsequent inserts
      }
  }

  namespace detail {
    template<bool doit> struct maybe_erase {
      template<class Algorithms>
      static void apply (typename Algorithms::container &
                         , typename Algorithms::index_param
                         , typename Algorithms::index_param)
      {
        PyErr_SetString (PyExc_TypeError
                         , "container does not support item deletion");

        boost::python::throw_error_already_set ();
      }
    };

    template<> struct maybe_erase<true> {
      template<class Algorithms>
      static void apply (typename Algorithms::container &c
                         , typename Algorithms::index_param from
                         , typename Algorithms::index_param to)
      {
        Algorithms::erase_range (c, from, to);
      }
    };
  }

  template<typename Algorithms>
  void int_slice_helper<Algorithms>::erase_remaining () const
  {
    if (mSlice.step() != 1)
      {
        PyErr_SetString (PyExc_ValueError
                         , "attempt to delete via extended slice");

        boost::python::throw_error_already_set ();
      }

    else
      {
        detail::maybe_erase<container_traits::has_erase>
          ::template apply<Algorithms> (*mPtr, mPos, mSlice.stop());
      }
  }
} } }

#endif // int_slice_helper_rmg_20031013_included