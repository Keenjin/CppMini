#pragma once
#include <memory>
#include "macros.h"
#include "lock.h"
#include "ref_counted.h"

namespace utils {

	template<typename T>
	class Singleton : public utils::RefCountedThreadSafe<Singleton<T>> {
	public:
		template<typename ...Args>
		static utils::scoped_refptr<T> GetInstance(Args&&... args) {
			if (!obj) {
				utils::AutoLock lock(lock_obj);
				if (nullptr == obj) {
					obj = utils::MakeRefCounted(std::forward<Args>(args)...);
				}
			}
			return obj;
		}

		static void DestroyInstance() {
			if (obj) {
				obj.release();
			}
		}

	protected:
		explicit Singleton() = default;
		virtual ~Singleton() = default;

	private:
		static utils::Lock lock_obj;
		static utils::scoped_refptr<T> obj;

		DISALLOW_COPY_AND_ASSIGN(Singleton<T>);
	};
}