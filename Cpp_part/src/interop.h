#pragma once

#include <entrypoint.h>

namespace interop
{
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