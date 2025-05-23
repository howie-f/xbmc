/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

/**
 * Defining LOG_LIFECYCLE_EVENTS will log all instantiations, deletions
 *  and also reference countings (increments and decrements) that take
 *  place on any Addon* class.
 *
 * Comment out (or uncomment out) to change the setting.
 */
//#define LOG_LIFECYCLE_EVENTS

/**
 * Defining XBMC_ADDON_DEBUG_MEMORY will make the Acquire and Release
 *  methods virtual allow the developer to overload them in a sub-class
 *  and set breakpoints to aid in debugging. It will also cause the
 *  reference counting mechanism to never actually delete any AddonClass
 *  instance allowing for the tracking of more references to (supposedly)
 *  deallocated classes.
 *
 * Comment out (or uncomment out) to change the setting.
 */
//#define XBMC_ADDON_DEBUG_MEMORY

#include "AddonString.h"

#include <mutex>
#ifdef XBMC_ADDON_DEBUG_MEMORY
#include "utils/log.h"
#endif
#include "AddonUtils.h"

#include <atomic>
#include <typeindex>

namespace XBMCAddon
{
  class LanguageHook;

  /**
   * This class is the superclass for all reference counted classes in the api.
   * It provides a means for the bindings to handle all api objects generically.
   *
   * It also provides some means for debugging "lifecycle" events (see the above
   *  description of LOG_LIFECYCLE_EVENTS).
   *
   * If a scripting language bindings require specific handling there is a
   *  hook to add in these language specifics that can be set here.
   */
  class AddonClass : public CCriticalSection
  {
  private:
    mutable std::atomic<long> refs;
    bool m_isDeallocating = false;

    // no copying
    inline AddonClass(const AddonClass&) = delete;

#ifdef XBMC_ADDON_DEBUG_MEMORY
    bool isDeleted;
#endif

  protected:
    LanguageHook* languageHook;

    /**
     * This method is meant to be called from the destructor of the
     *  lowest level class.
     *
     * It's virtual because it's a convenient place to receive messages that
     *  we're about to go be deleted but prior to any real tear-down.
     *
     * Any overloading classes need to remember to pass the call up the chain.
     */
    virtual void deallocating()
    {
      std::unique_lock lock(*this);
      m_isDeallocating = true;
    }

    /**
     * This is meant to be called during static initialization and so isn't
     * synchronized.
     */
    static short getNextClassIndex();

  public:
    AddonClass();
    virtual ~AddonClass();

    inline const char* GetClassname() const { return typeid(*this).name(); }
    inline LanguageHook* GetLanguageHook() { return languageHook; }

    /**
     * This method should be called while holding a Synchronize
     *  on the object. It will prevent the deallocation during
     *  the time it's held.
     */
    bool isDeallocating() { XBMC_TRACE; return m_isDeallocating; }

    static short getNumAddonClasses();

#ifdef XBMC_ADDON_DEBUG_MEMORY
    virtual
#else
    inline
#endif
    void Release() const
#ifndef XBMC_ADDON_DEBUG_MEMORY
    {
      long ct = --refs;
#ifdef LOG_LIFECYCLE_EVENTS
      CLog::Log(LOGDEBUG, "NEWADDON REFCNT decrementing to {} on {} 0x{:x}", ct, GetClassname(),
                (long)(((void*)this)));
#endif
      if(ct == 0)
        delete this;
    }
#else
    ;
#endif


#ifdef XBMC_ADDON_DEBUG_MEMORY
    virtual
#else
    inline
#endif
    void Acquire() const
#ifndef XBMC_ADDON_DEBUG_MEMORY
    {
#ifdef LOG_LIFECYCLE_EVENTS
      CLog::Log(LOGDEBUG, "NEWADDON REFCNT incrementing to {} on {} 0x{:x}", ++refs, GetClassname(),
                (long)(((void*)this)));
#else
      ++refs;
#endif
    }
#else
    ;
#endif

#define refcheck
    /**
     * This class is a smart pointer for a Referenced class.
     */
    template <class T> class Ref
    {
      T * ac;
    public:
      inline Ref() : ac(NULL) {}
      inline Ref(const T* _ac) : ac(const_cast<T*>(_ac)) { if (ac) ac->Acquire(); refcheck; }

      // copy semantics
      inline Ref(Ref<T> const & oref) : ac(const_cast<T*>(oref.get())) { if (ac) ac->Acquire(); refcheck; }
      template<class O> inline Ref(Ref<O> const & oref) : ac(static_cast<T*>(oref.get())) { if (ac) ac->Acquire(); refcheck; }

      /**
       * operator= should work with either another smart pointer or a pointer since it will
       * be able to convert a pointer to a smart pointer using one of the above constructors.
       *
       * Note: There is a trick here. The temporary variable is necessary because otherwise the
       * following code will fail:
       *
       * Ref<T> ptr = new T;
       * ptr = ptr;
       *
       * What happens without the tmp is the dereference is called first so the object ends up
       * deleted and then the reference happens on a deleted object. The order is reversed
       * in the following.
       *
       * Note: Operator= is ambiguous if you define both an operator=(Ref<T>&) and an operator=(T*). I'm
       * opting for the route the boost took here figuring it has more history behind it.
       */
      inline Ref<T>& operator=(Ref<T> const & oref)
      { T* tmp = ac; ac = const_cast<T*>(oref.get()); if (ac) ac->Acquire(); if (tmp) tmp->Release(); refcheck; return *this; }

      inline T* operator->() const { refcheck; return ac; }

      /**
       * This operator doubles as the value in a boolean expression.
       */
      inline operator T*() const { refcheck; return ac; }
      inline T* get() const { refcheck; return ac; }
      inline T& getRef() const { refcheck; return *ac; }

      inline ~Ref() { refcheck; if (ac) ac->Release(); }
      inline bool isNull() const { refcheck; return ac == NULL; }
      inline bool isNotNull() const { refcheck; return ac != NULL; }
      inline bool isSet() const { refcheck; return ac != NULL; }
      inline bool operator!() const { refcheck; return ac == NULL; }
      inline bool operator==(const AddonClass::Ref<T>& oref) const { refcheck; return ac == oref.ac; }
      inline bool operator<(const AddonClass::Ref<T>& oref) const { refcheck; return ac < oref.ac; } // std::set semantics

      // This is there only for boost compatibility
      template<class O> inline void reset(Ref<O> const & oref) { refcheck; (*this) = static_cast<T*>(oref.get()); refcheck; }
      template<class O> inline void reset(O * oref) { refcheck; (*this) = static_cast<T*>(oref); refcheck; }
      inline void reset() { refcheck; if (ac) ac->Release(); ac = NULL; }
    };

  };
}
