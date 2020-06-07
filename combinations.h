#ifndef __combinations_h__
#define __combinations_h__



#ifdef __cplusplus
extern "C" {
#endif


typedef int (*combination_callback)( void *in_context, void **in_objects, int in_M );


// Given an array of in_N objects each of size in_size, generate all
// combinations of in_M objects, calling in_callback once for each
// new combination.  The arguments to combination_callback are an
// array of pointers to objects in the in_objects array, and in_M, the
// number of objects in the combination array.  The in_callback function
// returns false to indicate that it wants another combination, or true
// to indicate that processing should be aborted.
void combinations(
	const void *in_objects,					// array of objects
	int in_size,							// size of each object
	int in_N,								// number of objects in array
	int in_M,								// number of objects to choose
	combination_callback in_callback,		// callback to call
	void *in_context );						// callback context


#ifdef __cplusplus
}
#endif



#endif // __combinations_h__
