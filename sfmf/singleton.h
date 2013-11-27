#pragma once
/*
  ==============================================================================

   This file is part of the S.F.Tracker
   Copyright 2005-7 by Satoshi Fujiwara.

   S.F.Tracker can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   S.F.Tracker is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with S.F.Tracker; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
   Boston, MA 02111-1307 USA

  ==============================================================================
*/
/** @file
 *  @brief 
 *  @author S.F. (Satoshi Fujiwara)
 */
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
namespace sf {
    template<typename BaseClass,template <class> class PointerType = boost::shared_ptr > struct singleton 
    {
        typedef PointerType<BaseClass> ptr;
        friend  BaseClass;
 
        static ptr& instance()
        {
            boost::call_once(init,flag_);
            return instance_;
        };


        singleton(){};
    private:
        singleton(const singleton& );
		static void init(){instance_.reset(new BaseClass);};
        static PointerType<BaseClass> instance_;
        static boost::once_flag flag_;
    };

    template<class BaseClass,template <class> class PointerType> boost::once_flag singleton<BaseClass,PointerType>::flag_ = BOOST_ONCE_INIT;
    template<class BaseClass,template <class> class PointerType> PointerType<BaseClass> singleton<BaseClass,PointerType>::instance_;
};


