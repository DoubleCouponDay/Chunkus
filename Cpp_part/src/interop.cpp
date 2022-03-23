#include "interop.h"

#if defined(WIN32) || defined(_WIN32)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#include <iostream>
#endif



namespace interop
{
	typedef void(*interop_action)();
	typedef test_struct(*interop_test_call)();
	typedef void(*interop_test_action)(test_struct*);
	typedef int(*interop_return_action)(vectorizer_data data);
	typedef algorithm_progress(*interop_do_vectorize)(vectorizer_data);
	typedef void(*interop_algorithm_action)(algorithm_progress*);

	interop_action epic_exported_function;
	interop_test_call get_test_struct;
	interop_test_action free_test_struct;

	interop_return_action do_the_vectorize;

	interop_do_vectorize begin_vectorization;
	interop_algorithm_action step_vectorization;
	interop_algorithm_action reverse_vectorization;
	interop_algorithm_action complete_vectorization;
}

#if defined(WIN32) || defined(_WIN32)
HMODULE vecLib = NULL;
#endif

void interop::release_shared_lib()
{
#if defined(WIN32) || defined(_WIN32)
	if (vecLib == NULL || vecLib == INVALID_HANDLE_VALUE)
		return;

	FreeLibrary(vecLib);
#endif
}

void interop::hot_reload()
{
#if defined(WIN32) || defined(_WIN32)
	if (vecLib != NULL && vecLib != INVALID_HANDLE_VALUE)
		FreeLibrary(vecLib);
	
	vecLib = LoadLibrary("vec");

	if (vecLib == NULL || vecLib == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Unable to load vec.dll (does it exist?)!" << std::endl;
		exit(1);
	}

	interop::epic_exported_function = (interop_action)GetProcAddress(vecLib, "epic_exported_function");
	if (!interop::epic_exported_function)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::get_test_struct = (interop_test_call)GetProcAddress(vecLib, "get_test_struct");
	if (!interop::get_test_struct)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::free_test_struct = (interop_test_action)GetProcAddress(vecLib, "free_test_struct");
	if (!interop::free_test_struct)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::do_the_vectorize = (interop_return_action)GetProcAddress(vecLib, "do_the_vectorize");
	if (!interop::do_the_vectorize)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::begin_vectorization = (interop_do_vectorize)GetProcAddress(vecLib, "begin_vectorization");
	if (!interop::begin_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::step_vectorization = (interop_algorithm_action)GetProcAddress(vecLib, "step_vectorization");
	if (!interop::step_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::reverse_vectorization = (interop_algorithm_action)GetProcAddress(vecLib, "reverse_vectorization");
	if (!interop::reverse_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::complete_vectorization = (interop_algorithm_action)GetProcAddress(vecLib, "complete_vectorization");
	if (!interop::complete_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	std::cout << "vec.dll has been hot reloaded!" << std::endl;

#endif
}

void dieIfIllegal()
{
#if defined(WIN32) || defined(_WIN32)
	if (vecLib == NULL || vecLib == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Can not call dynamic function that hasn't been loaded!" << std::endl;
		exit(1);
	}
#endif
}

test_struct interop::getTestStruct()
{
	dieIfIllegal();
	return interop::get_test_struct();
}

void interop::freeTestStruct(test_struct* t)
{
	dieIfIllegal();
	interop::free_test_struct(t);
}

int interop::doTheVectorize(vectorizer_data data)
{
	dieIfIllegal();
	return interop::do_the_vectorize(data);
}

algorithm_progress interop::beginVectorization(vectorizer_data data)
{
	dieIfIllegal();
	return interop::begin_vectorization(data);
}

void interop::stepVectorization(algorithm_progress* progress)
{
	dieIfIllegal();
	interop::step_vectorization(progress);
}

void interop::reverseVectorization(algorithm_progress* progress)
{
	dieIfIllegal();
	interop::reverse_vectorization(progress);
}

void interop::completeVectorization(algorithm_progress* progress)
{
	dieIfIllegal();
	interop::complete_vectorization(progress);
}
