#include "test_v8_terminate.h"

const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

std::string ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(try_catch->Exception());
	const char* exception_string = ToCString(exception);
	v8::Local<v8::Message> message = try_catch->Message();
	std::string error_msg;
	char str_error[4096];
	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		//sprintf_(str_error, "%s", exception_string);
		error_msg.append(str_error);
	}
	else {
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(isolate->GetCurrentContext());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber(context).FromJust();
		//sprintf(str_error, "%s:%i: %s", filename_string, linenum, exception_string);
		error_msg.append(str_error);
		// Print line of source code.
		v8::String::Utf8Value sourceline(
			message->GetSourceLine(context).ToLocalChecked());
		const char* sourceline_string = ToCString(sourceline);

		//sprintf(str_error, "%s", sourceline_string);
		error_msg.append(str_error);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn(context).FromJust();
		for (int i = 0; i < start; i++) {
			error_msg.append(" ");
		}
		int end = message->GetEndColumn(context).FromJust();
		for (int i = start; i < end; i++) {
			error_msg.append("^");
		}

		v8::Local<v8::Value> stack_trace_string;
		if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
			stack_trace_string->IsString() &&
			v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
			v8::String::Utf8Value stack_trace(stack_trace_string);
			const char* stack_trace_string = ToCString(stack_trace);
//			sprintf(str_error, "%s", stack_trace_string);
			error_msg.append(str_error);
		}
	}
	//LOG_ERROR("V8ErrorTrace:%s", error_msg.c_str());
	return error_msg;
}
void CallBackLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
	printf("CallBackLog");
}

v8::Isolate::CreateParams create_params_;
bool test_v8(const char *path) {
	v8::Platform* platform_ = nullptr;
	v8::V8::InitializeICUDefaultLocation(path);
	v8::V8::InitializeExternalStartupData(path);
	platform_ = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform_);
	if (!v8::V8::Initialize()) {
		return false;
	}
	create_params_.array_buffer_allocator =
		v8::ArrayBuffer::Allocator::NewDefaultAllocator();

	v8::Isolate* isolate_ = v8::Isolate::New(create_params_);;

	v8::Isolate::Scope isolate_scope(isolate_);
	v8::HandleScope handle_scope(isolate_);

	v8::TryCatch try_catch(isolate_);

	v8::Local<v8::Context> context;
	do {
		v8::Local<v8::ObjectTemplate> global_ = v8::ObjectTemplate::New(isolate_);

		global_->Set(
			v8::String::NewFromUtf8(isolate_, "callBackLog", v8::NewStringType::kNormal)
			.ToLocalChecked(),
			v8::FunctionTemplate::New(isolate_, CallBackLog));
		context = v8::Context::New(isolate_, NULL, global_);
	} while (false);

	v8::Context::Scope context_scope(context);

	//std::string code = "    var start=new Date().getTime(); \
		while (true) if (new Date().getTime() - start>3000) break; ";

	std::string code = "callBackLog();";
	v8::Local<v8::String> v8src = v8::String::NewFromUtf8(isolate_, code.c_str());
	v8::Local<v8::Script> compiled_script;

	if (!v8::Script::Compile(context, v8src).ToLocal(&compiled_script)) {
		std::string err_msg = ReportException(isolate_, &try_catch);
		printf("%s", err_msg.c_str());
		return false;
	}

	auto result = compiled_script->Run(context).ToLocalChecked();
	v8::String::Utf8Value utf8(result);
	printf("%s\n", *utf8);

	return true;
}

int main_v8(const char *path) {
	//std::thread thread(test_v8);
	test_v8(path);
	return 0;
}