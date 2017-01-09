#ifndef VECTOR_H_
#define VECTOR_H_


/* Because C doesn't have STL, we will be invoking our own vector struct
 * that we hope would aid us in the future projects. Because C also
 * doesn't have templates, we will create multiple vectors for each type.
 */

#define VECTOR_NOT_FOUND -1

/* * * * * * * * * * * * * *
 *      String struct      *
 * * * * * * * * * * * * * */
typedef struct {
  char* char_array;
  int length;
} String;

/* String constructor. Will not make this a pointer. */
String String_new(char* char_array);

/* String destructor. */
void String_free(String* target_string);

/* Only appending one character at a time. */
void string_append(String* target_string, const char char_to_append);

/* Producing substring from a string. */
String string_substr(String target_string, int start_idx, int end_idx);

/* No boolean type in C. 1 is true, 0 is false. */
int string_is_equal(const String self, const String compared_string);



/* * * * * * * * * * * * * *
 *   VectorString struct   *
 * * * * * * * * * * * * * */
typedef struct _VectorString VectorString;
typedef struct _VectorString {
  String* string_array;
  int length;

  /* Function pointer for push_back. Appends to end of vector. */
  void (*push_back)(VectorString*, const String);

  /* Function pointer for find. Returns index of the found string.
   * Else, it returns -1.
   */
  int (*find)(VectorString*, const String);
} VectorString;

/* VectorString constructor. */
VectorString VectorString_new(String* string_array, int length);

/* VectorString destructor */
void VectorString_free(VectorString* target_vector);

#endif /* VECTOR_H_ */