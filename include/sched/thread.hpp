#pragma once


#include <types.hpp>
#include <util/linked_list.hpp>


namespace sched
{
	class process;

	class thread : public util::list_node
	{
		public:
			thread ();

		private:
			process &proc;
	};
}
