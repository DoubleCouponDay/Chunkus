#pragma once

#include "../../C_part/src/entrypoint.h"

#include <string>
#include <vector>

#ifdef VECG_INTEROP
#if defined(WIN32) || defined(_WIN32)
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif
#include <Windows.h>
	typedef FARPROC(*getProcedureAddress)(HMODULE hModule, LPCSTR lpProcName);
	typedef HMODULE(*openSharedLib)(LPCSTR lpLibName);
	typedef BOOL(*closeSharedLib)(HMODULE module);
#else
#include <dlfcn.h>
#include <functional>
	typedef void*(*getProcedureAddress)(void* handle, const char* name);
	typedef decltype(std::bind(dlopen, std::placeholders::_1, 0)) openSharedLib;
	typedef int(*closeSharedLib)(void* handle);
#endif
#endif

class interop {
	private:
#ifdef VECG_INTEROP
	getProcedureAddress get_procedure_address;
	openSharedLib open_shared_lib;
	closeSharedLib close_shared_lib;
	gui_images* (*gui_vectorize)(vectorizer_data input);

	#if defined(WIN32) || defined(_WIN32)
		HMODULE vecLib;		
	#else
		void* vecLib;
	#endif

	bool isBad();
	void dieIfIllegal();
	void load_shared_lib();
	std::vector<std::string> getLibNames() const;
#endif
	std::string exe_folder;

	public:
	interop();
	void release_shared_lib();
	void hot_reload(); // Only works on windows currently

	void setExeFolder(std::string input_exefolder);

	interop& operator=(interop&& other);
};