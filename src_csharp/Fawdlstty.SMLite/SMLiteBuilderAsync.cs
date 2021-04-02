﻿using Fawdlstty.SMLite.ItemStruct;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fawdlstty.SMLite {
	public class SMLiteBuilderAsync<TState, TTrigger> where TState : IComparable where TTrigger : Enum {
		public _SMLite_ConfigStateAsync<TState, TTrigger> Configure (TState state) {
			if (m_builded_index > 0)
				throw new Exception ("shouldn't configure builder after builded.");
			if (m_states.ContainsKey (state))
				throw new Exception ("state is already exists.");
			var _state = new _SMLite_ConfigStateAsync<TState, TTrigger> { State = state };
			m_states.Add (state, _state);
			return _state;
		}

		public SMLiteAsync<TState, TTrigger> Build (TState init_state) {
			if (m_builded_index == 0) {
				lock (SMLiteAsync<TState, TTrigger>.s_cfg_states_group) {
					m_builded_index = ++SMLiteAsync<TState, TTrigger>.s_cfg_states_group_index;
					SMLiteAsync<TState, TTrigger>.s_cfg_states_group.Add (m_builded_index, m_states);
				}
			}
			return new SMLiteAsync<TState, TTrigger> (init_state, m_builded_index);
		}

		Dictionary<TState, _SMLite_ConfigStateAsync<TState, TTrigger>> m_states = new Dictionary<TState, _SMLite_ConfigStateAsync<TState, TTrigger>> ();
		int m_builded_index = 0;
	}
}
