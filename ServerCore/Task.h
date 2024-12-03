#pragma once

namespace ServerCore
{
	template<typename T>
	concept IsSptr = requires(T t) {
		{ t.IncRef() } -> std::convertible_to<void*>; 
		{ t.release() } -> std::convertible_to<void>;
	};

	template<typename T>
	concept IsNotMemFunc = !std::is_member_function_pointer_v<T> && !std::is_member_function_pointer_v<std::decay_t<T>>;

	class Task
	{
	public:
		Task()noexcept :argPtr{ nullptr } {}
		Task(const Task& other)noexcept
			: argPtr{ std::exchange(other.argPtr,nullptr) }
			, m_fpTask{ other.m_fpTask }
		{
		}
		void operator=(const Task& other)noexcept
		{
			if (this != &other)
			{
				argPtr = other.argPtr;
				m_fpTask = other.m_fpTask;
			}
			other.argPtr = nullptr;
		}
		Task(Task&& other)noexcept
			: argPtr{ std::exchange(other.argPtr,nullptr) }
			, m_fpTask{ other.m_fpTask }
		{
		}
		void operator=(Task&& other)noexcept
		{
			if (this != &other)
			{
				argPtr = other.argPtr;
				m_fpTask = other.m_fpTask;
			}
			other.argPtr = nullptr;
		}
		inline constexpr ~Task()noexcept {
			if (argPtr)
			{
				m_fpTask(argPtr, false);
				argPtr = nullptr;
			}
		}
		void swap(Task& other)noexcept { std::swap(*this, other); }

		template<typename T, typename U, typename Ret, typename... Args>
			requires std::derived_from<std::remove_pointer_t<std::decay_t<U>>, T> || (IsSptr<U> && std::derived_from<typename U::Type, T>)
		struct CallBack
		{
			Ret(T::* const memFunc)(Args...);
			mutable std::tuple<std::decay_t<Args>...> args;
			const std::decay_t<U> memFuncCaller;
			constexpr CallBack(Ret(T::* const memFunc_)(Args...), U&& memFuncInstance, Args&&... args_)noexcept
				: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ std::forward<U>(memFuncInstance) } {
			}

			inline constexpr void operator()()const noexcept
			{
				if constexpr (std::is_pointer_v<std::decay_t<U>>)
					invokeMemberFunction(memFunc, static_cast<T* const>(memFuncCaller), std::move(args));
				else
					invokeMemberFunction(memFunc, static_cast<T* const>(memFuncCaller.get()), std::move(args));
			}
		};

		template<typename T, typename U, typename Ret, typename... Args>
			requires std::derived_from<std::remove_pointer_t<std::decay_t<U>>, T> || (IsSptr<U> && std::derived_from<typename U::Type, T>)
		constexpr Task(Ret(T::* const memFunc)(Args...), U&& memFuncInstance, Args&&... args)noexcept
			: argPtr{ xnew<CallBack<T,U,Ret,Args...>>(memFunc,std::forward<U>(memFuncInstance),std::forward<Args>(args)...) }
			, m_fpTask{ [](const void* const callBackPtr_, const bool bExecute)noexcept 
				{
					if (bExecute)
						static_cast<const CallBack<T,U,Ret,Args...>* const>(callBackPtr_)->operator()();
					xdelete_sized<CallBack<T, U, Ret, Args...>>(static_cast<CallBack<T, U, Ret, Args...>* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack<T, U, Ret, Args...>));
			} }
		{
		}

		template<typename Func, typename... Args> requires std::invocable<Func, Args...>
		struct CallBackGeneric
		{
			mutable Func fp;
			mutable std::tuple<std::decay_t<Args>...> args;
			constexpr CallBackGeneric(Func&& fp_, Args&&... args_)noexcept
				: fp{ std::forward<Func>(fp_) }, args{ std::forward<Args>(args)... } {
			}
			inline constexpr void operator()()const noexcept
			{
				std::apply(std::move(fp), std::move(args));
			}
		};

		template<typename Func, typename... Args> requires std::invocable<Func, Args...> && IsNotMemFunc<Func>
		constexpr Task(Func&& fp, Args&&... args)noexcept
			: argPtr{ xnew<CallBackGeneric<Func,Args...>>(std::forward<Func>(fp), std::forward<Args>(args)...) }
			, m_fpTask{ [](const void* const callBackPtr_, const bool bExecute)noexcept 
				{
					if (bExecute)
						static_cast<const CallBackGeneric<Func,Args...>* const>(callBackPtr_)->operator()();
					xdelete_sized<CallBackGeneric<Func, Args...>>(static_cast<CallBackGeneric<Func, Args...>* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBackGeneric<Func, Args...>));
				} }
		{
			static_assert(!std::is_member_pointer_v<Func>);
			static_assert(!std::is_member_pointer_v<std::decay_t<Func>>);
		}
		inline constexpr void ExecuteTask()const noexcept { m_fpTask(argPtr, true); argPtr = nullptr; }
	private:
		mutable void* argPtr;
		void(*m_fpTask)(const void* const, const bool)noexcept;
	private:
		template<typename Function, typename T, typename Tuple, size_t... I>
		constexpr static inline const auto CallFunctionWithTuple(const Function func, T&& obj, Tuple&& tup, std::index_sequence<I...>) noexcept {
			return (*std::forward<T>(obj).*func)(std::get<I>(std::forward<Tuple>(tup))...);
		}

		template<typename Function, typename T>
		constexpr static inline const auto CallFunctionWithTuple(const Function func, T&& obj, std::tuple<>&&, std::index_sequence<>) noexcept {
			return (*std::forward<T>(obj).*func)();
		}
	public:
		template<typename Function, typename T, typename Tuple, typename Indices = std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>>
		constexpr static inline const auto invokeMemberFunction(Function&& func, T&& obj, Tuple&& tup) noexcept {
			return CallFunctionWithTuple(std::forward<Function>(func), std::forward<T>(obj), std::forward<Tuple>(tup), Indices{});
		}
	};

	class TaskInvoker
	{
		template<typename T, typename Ret, typename... Args>
		struct CallBack
		{
			Ret(T::* const memFunc)(Args...);
			const std::tuple<std::decay_t<Args>...> args;
			constexpr CallBack(Ret(T::* const memFunc_)(Args...), Args&&... args_) noexcept
				: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... } {
			}

			inline constexpr const void Execute(void* const memFuncInstance) const noexcept
			{
				Task::invokeMemberFunction(memFunc, static_cast<T* const>(memFuncInstance), args);
			}
		};
	public:
		~TaskInvoker()noexcept { m_fpTask(argPtr, nullptr); }
	public:
		template<typename T, typename Ret, typename... Args>
		constexpr TaskInvoker(Ret(T::* const memFunc)(Args...), Args&&... args) noexcept
			: argPtr{ xnew<CallBack<T,Ret,Args...>>(memFunc, std::forward<Args>(args)...) }
			, m_fpTask{ [](const void* const callBackPtr_, void* const memFuncInstance) noexcept
				{
					if (memFuncInstance)
						static_cast<const CallBack<T,Ret,Args...>* const>(callBackPtr_)->Execute(memFuncInstance);
					else
						xdelete_sized<CallBack<T, Ret, Args...>>(static_cast<CallBack<T, Ret, Args...>*const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack<T, Ret, Args...>));
			} }
		{
		}
	public:
		inline constexpr void InvokeTask(void* const memFuncInstance)const noexcept { m_fpTask(argPtr, memFuncInstance); }
	private:
		void* const argPtr;
		void (*const m_fpTask)(const void* const, void* const) noexcept;
	};
}

