// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"

namespace Private
{
	template <typename UserClass, ESPMode SPMode, typename FuncType, typename UserPolicy, typename FunctorType>
	class TSharedPtrDelegateImpl;

	template <typename UserClass, ESPMode SPMode, typename RetValType, typename... ParamTypes, typename UserPolicy, typename FunctorType>
	class TSharedPtrDelegateImpl<UserClass, SPMode, RetValType(ParamTypes...), UserPolicy, FunctorType>
		: public TCommonDelegateInstanceState<RetValType(ParamTypes...), UserPolicy>
	{
	private:
		static_assert(std::is_same_v<FunctorType, typename TRemoveReference<FunctorType>::Type>, "FunctorType cannot be a reference");

		using Super    = TCommonDelegateInstanceState<RetValType(ParamTypes...), UserPolicy>;
		using ThisType = TSharedPtrDelegateImpl<UserClass, SPMode, RetValType(ParamTypes...), UserPolicy, FunctorType>;

	public:
		TSharedPtrDelegateImpl(const TSharedPtr<UserClass, SPMode>& InUserObject, const FunctorType& InFunctor)
			: Super()
			, UserObject(InUserObject)
			, Functor(InFunctor)
		{
		}

		TSharedPtrDelegateImpl(const TSharedPtr<UserClass, SPMode>& InUserObject, FunctorType&& InFunctor)
			: Super()
			, UserObject(InUserObject)
			, Functor(MoveTemp(InFunctor))
		{
		}

		// IDelegateInstance interface

	#if USE_DELEGATE_TRYGETBOUNDFUNCTIONNAME
		FName TryGetBoundFunctionName() const final
		{
			return NAME_None;
		}
	#endif

		UObject* GetUObject() const final
		{
			return nullptr;
		}

		const void* GetObjectForTimerManager() const final
		{
			return UserObject.Pin().Get();
		}

		uint64 GetBoundProgramCounterForTimerManager() const final
		{
			return 0;
		}

		virtual bool HasSameObject(const void* InUserObject) const final override
		{
			return UserObject.HasSameObject(InUserObject);
		}

		virtual bool IsSafeToExecute() const final override
		{
			return UserObject.IsValid();
		}

		// IBaseDelegateInstance interface
		virtual void CreateCopy(TDelegateBase<FThreadSafeDelegateMode>& Base) const final override
		{
			CreateDelegateInstanceImpl<ThisType>(Base, *this);
		}

		virtual void CreateCopy(TDelegateBase<FNotThreadSafeDelegateMode>& Base) const override
		{
			CreateDelegateInstanceImpl<ThisType>(Base, *this);
		}

		virtual void CreateCopy(TDelegateBase<FNotThreadSafeNotCheckedDelegateMode>& Base) const override
		{
			CreateDelegateInstanceImpl<ThisType>(Base, *this);
		}

		virtual RetValType Execute(ParamTypes... Params) const final override
		{
			// Verify that the user object is still valid.  We only have a weak reference to it.
			const TSharedPtr<UserClass, SPMode> SharedUserObject = UserObject.Pin();
			check(SharedUserObject.IsValid());

			return Functor(Forward<ParamTypes>(Params)...);
		}

		virtual bool ExecuteIfSafe(ParamTypes... Params) const final override
		{
			if (TSharedPtr<UserClass, SPMode> SharedUserObject = UserObject.Pin())
			{
				(void)Functor(Forward<ParamTypes>(Params)...);
				return true;
			}
			return false;
		}

		/**
		 * Creates a new shared pointer delegate binding for the given user object and lambda.
		 */
		FORCEINLINE static void Create(TDelegateBase<FDefaultDelegateUserPolicy::FThreadSafetyMode>& Base, const TSharedPtr<UserClass, SPMode>& InUserObjectRef, FunctorType&& InFunc)
		{
			CreateDelegateInstanceImpl<ThisType>(Base, InUserObjectRef, MoveTemp(InFunc));
		}

		/**
		 * Creates a new shared pointer delegate binding for the given user object and lambda.
		 * InUserObject has to derive from TSharedFromThis.
		 */
		FORCEINLINE static void Create(TDelegateBase<FDefaultDelegateUserPolicy::FThreadSafetyMode>& Base, UserClass* InUserObject, FunctorType&& InFunc)
		{
			// We expect the incoming InUserObject to derive from TSharedFromThis.
			const TSharedRef<UserClass, SPMode> UserObjectRef = StaticCastSharedRef<UserClass>(InUserObject->AsShared());
			Create(Base, UserObjectRef, MoveTemp(InFunc));
		}

	private:
	
		template <typename InstanceType, typename DelegateMode, typename... Args>
		static void CreateDelegateInstanceImpl(TDelegateBase<DelegateMode>& BaseDelegate, Args&&... ArgsIn)
		{
			new (TWriteLockedDelegateAllocation{ BaseDelegate }) InstanceType(Forward<Args>(ArgsIn)...);
		}

		// Context object - the validity of this object controls the validity of the lambda
		TWeakPtr<UserClass, SPMode> UserObject;

		// C++ functor
		// We make this mutable to allow mutable lambdas to be bound and executed.  We don't really want to
		// model the Functor as being a direct subobject of the delegate (which would maintain transivity of
		// const - because the binding doesn't affect the substitutability of a copied delegate.
		mutable std::remove_const_t<FunctorType> Functor;
	};

	template<class Lambda>
	struct TDelegateFromLambda : TDelegateFromLambda<decltype(&Lambda::operator())>
	{
	};

	template<typename TReturnType, typename TClass, typename... TArgs>
	struct TDelegateFromLambda<TReturnType(TClass::*)(TArgs...) const> : TDelegateFromLambda<TReturnType(TClass::*)(TArgs...)>
	{
	};

	template<typename TReturnType, typename TClass, typename... TArgs>
	struct TDelegateFromLambda<TReturnType(TClass::*)(TArgs...)>
	{
		using Type = TDelegate<TReturnType(TArgs...)>;

		template<typename UserClass, ESPMode Mode, typename TFunctor>
		using TDelegateImpl = TSharedPtrDelegateImpl<UserClass, Mode, TReturnType(TArgs...), FDefaultDelegateUserPolicy, TFunctor>;
	};
}

template<typename TLambda>
inline auto MakeLambdaDelegate(TLambda Lambda)
{
	return Private::TDelegateFromLambda<TLambda>::Type::CreateLambda(MoveTemp(Lambda));
}

template<typename T, typename TLambda>
inline auto MakeWeakObjectPtrDelegate(T* Ptr, TLambda Lambda)
{
	return Private::TDelegateFromLambda<TLambda>::Type::CreateWeakLambda(const_cast<std::remove_const_t<T>*>(Ptr), MoveTemp(Lambda));
}

template<typename TClass, ESPMode Mode, typename TLambda>
inline auto MakeWeakPtrDelegate(const TSharedPtr<TClass, Mode>& Object, TLambda Lambda)
{
	typename Private::TDelegateFromLambda<TLambda>::Type Delegate;
	Private::TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, Mode, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}

template<typename TClass, typename TLambda>
inline auto MakeWeakPtrDelegate(TClass* Object, TLambda Lambda)
{
	typename Private::TDelegateFromLambda<TLambda>::Type Delegate;
	Private::TDelegateFromLambda<TLambda>::template TDelegateImpl<TClass, ESPMode::ThreadSafe, TLambda>::Create(Delegate, Object, MoveTemp(Lambda));
	return Delegate;
}
