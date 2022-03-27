#pragma once

#include "../../C_part/src/entrypoint.h"

typedef void(*interop_action)();
typedef test_struct(*interop_test_call)();
typedef void(*interop_test_action)(test_struct*);
typedef int(*interop_return_action)(vectorizer_data data);
typedef algorithm_progress(*interop_do_vectorize)(vectorizer_data);
typedef void*(*interop_algorithm_action)(algorithm_progress*);

#if defined(WIN32) || defined(_WIN32)
	typedef interop_action(*getProcedureAddress)(HMODULE hModule, LTCSTR lpProcName);
	typedef HMODULE(*openSharedLib)(LTCSTR lpLibName);
	typedef void(*closeSharedLib)(HMODULE module);
#else
#include <dlfcn.h>
	typedef void*(*getProcedureAddress)(void* handle, const char* name);
	typedef decltype(std::bind(dlopen, std::placeholders::_1, 0)) openSharedLib;
	typedef int(*closeSharedLib)(void* handle);
#endif

class interop {
	private:
	interop_action epic_exported_function;
	getProcedureAddress get_procedure_address;
	openSharedLib open_shared_lib;
	closeSharedLib close_shared_lib;
	interop_test_call get_test_struct;
	interop_test_action free_test_struct;

	interop_return_action do_the_vectorize;

	interop_do_vectorize begin_vectorization;
	interop_algorithm_action step_vectorization;
	interop_algorithm_action reverse_vectorization;
	interop_algorithm_action complete_vectorization;

	#if defined(WIN32) || defined(_WIN32)
		HMODULE vecLib;		
	#else
		void* vecLib;
	#endif

	public:
	interop();
	void release_shared_lib();
	void hot_reload(); // Only works on windows currently

	test_struct getTestStruct();
	void freeTestStruct(test_struct* t);

	int doTheVectorize(vectorizer_data data);

	algorithm_progress beginVectorization(vectorizer_data data);
	void stepVectorization(algorithm_progress* progress);
	void reverseVectorization(algorithm_progress* progress);
	void completeVectorization(algorithm_progress* progress);
};