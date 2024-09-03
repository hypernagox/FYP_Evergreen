#pragma once

namespace ServerCore
{
	template<typename T>
	concept EnableSharedFromThis = requires(T t) {
		{ t.SharedFromThis() };
	};

	template<typename T>
	concept IsNotMemFunc = !std::is_member_function_pointer_v<T> && !std::is_member_function_pointer_v<std::decay_t<T>>;

	class Session;

	class Task
	{
	public:
		Task()noexcept :argPtr{ nullptr } {}
		Task(const Task& other)noexcept
			: argPtr{ std::exchange(other.argPtr,nullptr) }
			, m_fpTask{ other.m_fpTask }
			, m_fpTaskDeleter{ other.m_fpTaskDeleter }
		{
		}
		void operator=(const Task& other)noexcept
		{
			if (this != &other)
			{
				argPtr = other.argPtr;
				m_fpTask = other.m_fpTask;
				m_fpTaskDeleter = other.m_fpTaskDeleter;
			}
			other.argPtr = nullptr;
		}
		Task(Task&& other)noexcept
			: argPtr{ std::exchange(other.argPtr,nullptr) }
			, m_fpTask{ other.m_fpTask }
			, m_fpTaskDeleter{ other.m_fpTaskDeleter }
		{
		}
		void operator=(Task&& other)noexcept
		{
			if (this != &other)
			{
				argPtr = other.argPtr;
				m_fpTask = other.m_fpTask;
				m_fpTaskDeleter = other.m_fpTaskDeleter;
			}
			other.argPtr = nullptr;
		}
		inline constexpr ~Task()noexcept {
			if (argPtr)
			{
				m_fpTaskDeleter(argPtr);
				argPtr = nullptr;
			}
		}
		void swap(Task& other)noexcept { std::swap(*this, other); }
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...)noexcept;
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...)noexcept, const S_ptr<U>& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ memFuncInstance } {}

				inline constexpr const void operator()()const noexcept
				{
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			argPtr = xnew<CallBack>(memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...)noexcept;
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...)noexcept, S_ptr<U>&& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ std::move(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			argPtr = xnew<CallBack>(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...)noexcept, U* const memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...)noexcept;
				mutable std::tuple<std::decay_t<Args>...> args;
				T* const memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...)noexcept, U* const memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ static_cast<T* const>(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			argPtr = xnew<CallBack>(memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}

		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...), const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...);
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...), const S_ptr<U>& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ memFuncInstance } {}

				inline constexpr const void operator()()const noexcept
				{
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			argPtr = xnew<CallBack>(memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...), S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...);
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...), S_ptr<U>&& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ std::move(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			argPtr = xnew<CallBack>(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...), U* const memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...);
				mutable std::tuple<std::decay_t<Args>...> args;
				T* const memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...), U* const memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ static_cast<T* const>(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			argPtr = xnew<CallBack>(memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...> && IsNotMemFunc<Func>
		constexpr Task(Func&& fp, Args&&... args)noexcept
		{
			static_assert(!std::is_member_pointer_v<Func>);
			static_assert(!std::is_member_pointer_v<std::decay_t<Func>>);
			static_assert(IsNotMemFunc<Func>);
			struct CallBack
			{
				mutable Func fp;
				mutable std::tuple<std::decay_t<Args>...> args;
				constexpr CallBack(Func&& fp_, Args&&... args_)noexcept
					: fp{ std::forward<Func>(fp_) }, args{ std::forward<Args>(args)... } {}
				inline constexpr void operator()()const noexcept
				{
					std::apply(std::move(fp), std::move(args));
				}
			};
			argPtr = xnew<CallBack>(std::forward<Func>(fp), std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete_sized<CallBack>(static_cast<CallBack* const>(callBackPtr_), sizeof(CallBack)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {
				static_cast<const CallBack* const>(callBackPtr_)->operator()();
				xdelete_sized<CallBack>(static_cast<CallBack* const>(const_cast<void* const>(callBackPtr_)), sizeof(CallBack));
				};
		}
		inline constexpr void ExecuteTask()const noexcept { m_fpTask(argPtr); argPtr = nullptr; }
	private:
		mutable void* argPtr;
		void(*m_fpTask)(const void* const)noexcept;
		void(*m_fpTaskDeleter)(void* const)noexcept;
	private:
		template<typename Function, typename T, typename Tuple, size_t... I>
		constexpr static inline const auto CallFunctionWithTuple(const Function func, T&& obj, Tuple&& tup, std::index_sequence<I...>) noexcept {
			return (*std::forward<T>(obj).*func)(std::get<I>(std::forward<Tuple>(tup))...);
		}

		template<typename Function, typename T>
		constexpr static inline const auto CallFunctionWithTuple(const Function func, T&& obj, std::tuple<>&&, std::index_sequence<>) noexcept {
			return (*std::forward<T>(obj).*func)();
		}

		template<typename Function, typename T, typename Tuple, typename Indices = std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>>
		constexpr static inline const auto invokeMemberFunction(Function&& func, T&& obj, Tuple&& tup) noexcept {
			return CallFunctionWithTuple(std::forward<Function>(func), std::forward<T>(obj), std::forward<Tuple>(tup), Indices{});
		}
	};
}
