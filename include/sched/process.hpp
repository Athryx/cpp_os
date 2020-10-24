#pragma once


#include <types.hpp>
#include <sched/thread.hpp>
#include <mem/vmem.hpp>
#include <util/linked_list.hpp>
#include <util/linked_list2.hpp>


namespace sched
{
	class process : public util::list_node
	{
		public:
			process ();

		private:
			mem::addr_space addr_space;

			util::linked_list2<thread> threads;
	};


	extern sched::process proc_c;
}
