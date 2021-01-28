/*
* SMLite
* State machine library for C++ & C# & Python
* Author: Fawdlstty
* Version 0.1.5
* 
* Source Repository            <https://github.com/fawdlstty/SMLite>
* Report                       <https://github.com/fawdlstty/SMLite/issues>
* MIT License                  <https://opensource.org/licenses/MIT>
* Copyright (C) 2021 Fawdlstty <https://www.fawdlstty.com>
*/

#ifndef __SMLITE_HPP__
#define __SMLITE_HPP__

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <vector>



namespace Fawdlstty {

	class _SMLite_Exception: public std::exception {
	public:
		_SMLite_Exception (const char* _reason): m_reason (_reason) {}
		const char* what () const noexcept override { return m_reason; }

	private:
		const char* m_reason;
	};

	class _SMLite_ConfigItem {
		virtual void f () = 0;

	public:
		virtual ~_SMLite_ConfigItem () = default; // fix warning
	};

	template<typename TState, typename TTrigger>					class _SMLite_ConfigItem0;
	template<typename TState, typename TTrigger, typename... Args>	class _SMLite_ConfigItem1;
	template<typename TState, typename TTrigger>					class _SMLite_ConfigState;
	template<typename TState, typename TTrigger>					class SMLite;
	template<typename TState, typename TTrigger>					class SMLiteBuilder;



	//
	// trigger item
	//

	template<typename TState, typename TTrigger>
	class _SMLite_ConfigItem0: public _SMLite_ConfigItem {
		void f () override {}

	public:
		virtual ~_SMLite_ConfigItem0 () = default; // fix warning
		_SMLite_ConfigItem0 (TState state, TTrigger trigger, std::function<TState (TState, TTrigger)> callback)
			: m_state (state), m_trigger (trigger), m_callback (callback) {}
		TState _call () { return m_callback (m_state, m_trigger); }

	private:
		TState m_state;
		TTrigger m_trigger;
		std::function<TState (TState, TTrigger)> m_callback;
	};

	template<typename TState, typename TTrigger, typename... Args>
	class _SMLite_ConfigItem1: public _SMLite_ConfigItem {
		void f () override {}

	public:
		virtual ~_SMLite_ConfigItem1 () = default; // fix warning
		_SMLite_ConfigItem1 (TState state, TTrigger trigger, std::function<TState (TState, TTrigger, Args...)> callback)
			: m_state (state), m_trigger (trigger), m_callback (callback) {}
		TState _call (Args... args) { return m_callback (m_state, m_trigger, args...); }

	private:
		TState m_state;
		TTrigger m_trigger;
		std::function<TState (TState, TTrigger, Args...)> m_callback;
	};



	//
	// state item (include trigger groups)
	//

	template<typename TState, typename TTrigger>
	class _SMLite_ConfigState : public std::enable_shared_from_this<_SMLite_ConfigState<TState, TTrigger>> {
		friend class SMLite<TState, TTrigger>;
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> _try_add_trigger (TTrigger _trigger, _SMLite_ConfigItem *_ptr) {
			if (m_items.find (_trigger) != m_items.end ())
				throw _SMLite_Exception ("state is already has this trigger methods.");
			m_items [_trigger] = std::shared_ptr<_SMLite_ConfigItem> (_ptr);
			return this->shared_from_this ();
		}

	public:
		_SMLite_ConfigState (TState state) : m_state (state) {}
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> WhenFunc (TTrigger trigger, std::function<TState (TState, TTrigger)> callback) {
			return _try_add_trigger (trigger, new _SMLite_ConfigItem0<TState, TTrigger> (m_state, trigger, callback));
		}
		template<typename... Args>
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> WhenFunc (TTrigger trigger, std::function<TState (TState, TTrigger, Args...)> callback) {
			return _try_add_trigger (trigger, new _SMLite_ConfigItem1<TState, TTrigger, Args...> (m_state, trigger, callback));
		}
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> WhenAction (TTrigger trigger, std::function<void (TState, TTrigger)> callback) {
			return _try_add_trigger (trigger, new _SMLite_ConfigItem0<TState, TTrigger> (m_state, trigger, std::function<TState (TState, TTrigger)> ([callback] (TState state, TTrigger trigger) -> TState {
				callback (state, trigger);
				return state;
			})));
		}
		template<typename... Args>
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> WhenAction (TTrigger trigger, std::function<void (TState, TTrigger, Args...)> callback) {
			return _try_add_trigger (trigger, new _SMLite_ConfigItem1<TState, TTrigger, Args...> (m_state, trigger, std::function<TState (TState, TTrigger, Args...)> ([callback] (TState state, TTrigger trigger, Args... args) -> TState {
				callback (state, trigger, args...);
				return state;
			})));
		}
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> WhenChangeTo (TTrigger trigger, TState new_state) {
			return _try_add_trigger (trigger, new _SMLite_ConfigItem0<TState, TTrigger> (m_state, trigger, std::function<TState (TState, TTrigger)> ([new_state] (TState state, TTrigger trigger) -> TState {
				return new_state;
			})));
		}
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> WhenIgnore (TTrigger trigger) {
			std::function<TState (TState, TTrigger)> f = std::function<TState (TState, TTrigger)> ([] (TState state, TTrigger trigger) -> TState {
				return state;
			});
			return _try_add_trigger (trigger, new _SMLite_ConfigItem0<TState, TTrigger> (m_state, trigger, f));
		}
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> OnEntry (std::function<void ()> callback) {
			if (m_on_entry)
				throw _SMLite_Exception ("OnEntry is already have been set.");
			m_on_entry = callback;
			return this->shared_from_this ();
		}
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> OnLeave (std::function<void ()> callback) {
			if (m_on_leave)
				throw _SMLite_Exception ("OnLeave is already have been set.");
			m_on_leave = callback;
			return this->shared_from_this ();
		}

	private:
		bool _allow_trigger (TTrigger trigger) { return m_items.find (trigger) != m_items.end (); }
		TState _trigger (TTrigger trigger) {
			_SMLite_ConfigItem *_ptr0 = m_items [trigger].get ();
			if (_ptr0) {
				auto _ptr1 = dynamic_cast<_SMLite_ConfigItem0<TState, TTrigger>*> (_ptr0);
				if (_ptr1)
					return _ptr1->_call ();
			}
			throw _SMLite_Exception ("not match function found.");
		}
		template<typename... Args>
		TState _trigger (TTrigger trigger, Args... args) {
			_SMLite_ConfigItem *_ptr0 = m_items [trigger].get ();
			if (_ptr0) {
				auto _ptr1 = dynamic_cast<_SMLite_ConfigItem1<TState, TTrigger, Args...>*> (_ptr0);
				if (_ptr1)
					return _ptr1->_call (args...);
			}
			throw _SMLite_Exception ("not match function found.");
		}

		std::function<void ()> m_on_entry, m_on_leave;
		TState m_state;
		std::map<TTrigger, std::shared_ptr<_SMLite_ConfigItem>> m_items;
	};



	//
	// state machine (include state groups)
	//

	template<typename TState, typename TTrigger>
	class SMLite {
		friend class SMLiteBuilder<TState, TTrigger>;
		SMLite (TState init_state, std::shared_ptr<std::map<TState, std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>>>> _states)
			: m_state (init_state), m_states (_states) {}
	public:
		TState GetState () { return m_state; }
		void SetState (TState new_state) { m_state = new_state; }
		bool AllowTriggering (TTrigger trigger) {
			if (m_states->find (m_state) != m_states->end ())
				return (*m_states) [m_state]->_allow_trigger (trigger);
			return false;
		}
		void Triggering (TTrigger trigger) {
			if (!AllowTriggering (trigger))
				throw _SMLite_Exception ("current state cannot launch this trigger.");
			auto _p = (*m_states) [m_state];
			auto _state = _p->_trigger (trigger);
			if (m_state != _state) {
				if (_p->m_on_leave)
					_p->m_on_leave ();
				m_state = _state;
				_p = (*m_states) [m_state];
				if (_p->m_on_entry)
					_p->m_on_entry ();
			}
		}
		template<typename... Args>
		void Triggering (TTrigger trigger, Args... args) {
			if (!AllowTriggering (trigger))
				throw _SMLite_Exception ("current state cannot launch this trigger.");
			auto _p = (*m_states) [m_state];
			auto _state = _p->_trigger (trigger, args...);
			if (m_state != _state) {
				if (_p->m_on_leave)
					_p->m_on_leave ();
				m_state = _state;
				_p = (*m_states) [m_state];
				if (_p->m_on_entry)
					_p->m_on_entry ();
			}
		}

	private:
		TState m_state;
		std::shared_ptr<std::map<TState, std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>>>> m_states;
	};

	template<typename TState, typename TTrigger>
	class SMLiteBuilder {
	public:
		std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>> Configure (TState state) {
			if (m_builded)
				throw _SMLite_Exception ("shouldn't configure builder after builded.");
			if (m_states->find (state) != m_states->end ())
				throw _SMLite_Exception ("state is already exists.");
			auto _ptr = std::make_shared<_SMLite_ConfigState<TState, TTrigger>> (state);
			(*m_states) [state] = _ptr;
			return _ptr;
		}
		std::shared_ptr<SMLite<TState, TTrigger>> Build (TState init_state) {
			m_builded = true;
			return std::shared_ptr<SMLite<TState, TTrigger>> (new SMLite<TState, TTrigger> (init_state, m_states)); // fix compile error
			//return std::make_shared<SMLite<TState, TTrigger>> (init_state, m_states);
		}

	private:
		std::shared_ptr<std::map<TState, std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>>>> m_states
			= std::make_shared<std::map<TState, std::shared_ptr<_SMLite_ConfigState<TState, TTrigger>>>> ();
		bool m_builded = false;
	};
}

#endif //__SMLITE_HPP__