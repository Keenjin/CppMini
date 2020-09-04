#include "message_window.h"
#include <assert.h>

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

// Returns the HMODULE of the dll the macro was expanded in.
// Only use in cc files, not in h files.
#define CURRENT_MODULE() reinterpret_cast<HMODULE>(&__ImageBase)

namespace task_schedule {

	// static
	LRESULT CALLBACK WindowProc(HWND hwnd,
		UINT message,
		WPARAM wparam,
		LPARAM lparam) {
		MessageWindow* self =
			reinterpret_cast<MessageWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch (message) {
			// Set up the self before handling WM_CREATE.
		case WM_CREATE: {
			CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
			self = reinterpret_cast<MessageWindow*>(cs->lpCreateParams);
			LONG_PTR result = SetWindowLongPtr(hwnd, GWLP_USERDATA,
				reinterpret_cast<LONG_PTR>(self));
			break;
		}

		case WM_DESTROY: {
			SetLastError(ERROR_SUCCESS);
			LONG_PTR result = SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
			break;
		}
		}

		// Handle the message.
		if (self) {
			bool handle_self = true;
			LRESULT result = self->OnMsgProc(message, wparam, lparam, &handle_self);
			if (handle_self) return result;
		}

		return DefWindowProc(hwnd, message, wparam, lparam);
	}

	const wchar_t kMessageWindowClassName[] = L"CppMini_MessageWindow";

	class MessageWindow::WindowClass {
	public:
		WindowClass();
		~WindowClass();

		ATOM atom() { return atom_; }
		HINSTANCE instance() { return instance_; }

	private:
		ATOM atom_ = 0;
		HINSTANCE instance_ = CURRENT_MODULE();

		DISALLOW_COPY_AND_ASSIGN(WindowClass);
	};

	MessageWindow::WindowClass::WindowClass() {
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof(window_class);
		window_class.style = 0;
		window_class.lpfnWndProc = WindowProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = instance_;
		window_class.hIcon = nullptr;
		window_class.hCursor = nullptr;
		window_class.hbrBackground = nullptr;
		window_class.lpszMenuName = nullptr;
		window_class.lpszClassName = kMessageWindowClassName;
		window_class.hIconSm = nullptr;
		atom_ = RegisterClassEx(&window_class);
		if (atom_ == 0) {
			assert(false && "failed to register class");
		}
	}

	static MessageWindow::WindowClass window_class;

	MessageWindow::WindowClass::~WindowClass() {
		if (atom_ != 0) {
			BOOL result = UnregisterClass(MAKEINTATOM(atom_), instance_);
			assert(result);
		}
	}

	MessageWindow::MessageWindow(MessageWindow::Delegate* delegate)
		: delegate_(delegate) {

	}

	MessageWindow::~MessageWindow() {
		if (window_)
			::DestroyWindow(window_);
	}

	bool MessageWindow::Create(const std::wstring& window_name) {
		if (window_ != nullptr)
			return false;

		window_ =
			::CreateWindow(MAKEINTATOM(window_class.atom()), window_name.c_str(), 0, 0, 0, 0, 0,
				HWND_MESSAGE, nullptr, window_class.instance(), this);
		if (!window_) {
			return false;
		}

		return true;
	}

	LRESULT MessageWindow::OnMsgProc(
		UINT message,
		WPARAM wparam,
		LPARAM lparam,
		bool* handle_self) {
		if (message == WM_CREATE) {
			*handle_self = false;
			if (delegate_) delegate_->OnMainEntry();
		} else if (message == WM_DESTROY){
			*handle_self = false;
			if (delegate_) delegate_->OnMainExit();
		} else if (message == kMessageCommunition) {
			*handle_self = true;
			if (delegate_) {
				Task task = delegate_->GetNextTask();
				delegate_->TaskDone(task, delegate_->ScheduleTask(task));
			}
		} else {
			*handle_self = false;
		}

		return S_OK;
	}

	void MessageWindow::Submit() {
		PostMessage(window_, kMessageCommunition, NULL, NULL);
	}
}