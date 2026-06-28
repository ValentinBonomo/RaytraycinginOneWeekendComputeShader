#ifndef GALLIUM__OVERLOADED_H
#define GALLIUM__OVERLOADED_H
#pragma once

namespace ga
{
	template<typename... Ts>
	struct overloaded
		: Ts...
	{ using Ts::operator()...; };

	template<typename... Ts>
	overloaded(Ts...) -> overloaded<Ts...>;
}

#endif /* GALLIUM__OVERLOADED_H */
