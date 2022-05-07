#include "interop.h"

#if defined(WIN32) || defined(_WIN32)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

#include <iostream>

#include <functional>

interop::interop()
#if defined(__linux__)
	: open_shared_lib(std::bind(dlopen, std::placeholders::_1, RTLD_NOW|RTLD_GLOBAL))
#endif
 {
	vecLib = NULL;
	#if defined(WIN32) || defined(_WIN32)
		get_procedure_address = GetProcAddress;
		open_shared_lib = LoadLibrary;
		close_shared_lib = FreeLibrary;
	#else
		get_procedure_address = dlsym;
		close_shared_lib = dlclose;
	#endif
}

void interop::setExeFolder(std::string input_exefolder)
{
	exe_folder = input_exefolder;
}

void interop::release_shared_lib()
{
	if (isBad())
		return;

	close_shared_lib(vecLib);
}

#include <filesystem>

void interop::load_shared_lib() {
	release_shared_lib();

	auto libnames = getLibNames();

	std::cout << "Trying to load shared vec library..." << std::endl;

	for (int i = 0; i < libnames.size(); ++i)
	{
		auto& name = libnames[i];
		vecLib = open_shared_lib(name.c_str());

		if (!isBad())
			break;
	}

	if (isBad())
	{
		std::string err = "";
#ifdef __linux__
		err = dlerror();
		std::cerr << "Linux DL err: " << err << std::endl;
#endif

		if (libnames.empty())
		{
			std::cerr << "Unable to load shared vec library. getLibNames() returned empty vector (something went wrong)" << std::endl;
			exit(1);
		}

		std::cerr << "Unable to load shared library vec. Tried:" << std::endl;
		for (int i = 0; i < libnames.size(); ++i)
			std::cerr << "\t" << libnames[i] << std::endl;
		exit(1);
	}
}

void interop::hot_reload()
{
	load_shared_lib();
	

	

	interop::epic_exported_function = (interop_action)get_procedure_address(vecLib, "epic_exported_function");
	if (!interop::epic_exported_function)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::get_test_struct = (interop_test_call)get_procedure_address(vecLib, "get_test_struct");
	if (!interop::get_test_struct)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::free_test_struct = (interop_test_action)get_procedure_address(vecLib, "free_test_struct");
	if (!interop::free_test_struct)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::do_the_vectorize = (interop_return_action)get_procedure_address(vecLib, "do_the_vectorize");
	if (!interop::do_the_vectorize)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::begin_vectorization = (interop_do_vectorize)get_procedure_address(vecLib, "begin_vectorization");
	if (!interop::begin_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::step_vectorization = (interop_algorithm_action)get_procedure_address(vecLib, "step_vectorization");
	if (!interop::step_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::reverse_vectorization = (interop_algorithm_action)get_procedure_address(vecLib, "reverse_vectorization");
	if (!interop::reverse_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	interop::complete_vectorization = (interop_algorithm_action)get_procedure_address(vecLib, "complete_vectorization");
	if (!interop::complete_vectorization)
	{
		std::cerr << "Unable to load functions from vec.dll!" << std::endl; 
		exit(1);
	}
	std::cout << "vec.dll has been hot reloaded!" << std::endl;
}

bool interop::isBad()
{
#if defined(WIN32) | defined(_WIN32)
	if (vecLib == INVALID_HANDLE_VALUE)
		return true;
#endif
	return vecLib == NULL;
}

void interop::dieIfIllegal()
{
	if (isBad())
	{
		std::cerr << "Can not call dynamic function that hasn't been loaded!" << std::endl;
		exit(1);
	}
}

std::vector<std::string> interop::getLibNames() const
{
	std::vector<std::filesystem::path> paths{};
	auto p = std::filesystem::path(exe_folder);
#if defined(WIN32) | defined(_WIN32)
	paths.emplace_back(p / "vec.dll");
	paths.emplace_back(p / "libvec.dll");
	paths.emplace_back("vec.dll");
	paths.emplace_back("libvec.dll");
#else
	paths.emplace_back(p / "libvec.so");
	paths.emplace_back(p / "vec.so");
	paths.emplace_back("libvec.so");
	paths.emplace_back("vec.so");
#endif

	std::vector<std::string> strings{ paths.size(), std::string() };

	for (int i = 0; i < paths.size(); ++i)
		strings[i] = paths[i].string();

	return strings;
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