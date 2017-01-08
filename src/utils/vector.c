#include "vector.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// String constructor. Will not make this a pointer.
String String_new(char* char_array) {
  String string;

  // Soft copy is fine for now.
  string.char_array = char_array;
  string.length = strlen(char_array);
  return string;
}

// String destructor.
void String_free(String* target_string) {
  free(target_string->char_array);
}

// Only appending one character at a time.
void string_append(String* target_string, const char char_to_append) {
  int new_length = target_string->length + 1;

  // Shallow copy here so they point to the same thing.
  char* new_char_array = target_string->char_array;
  new_char_array = realloc(new_char_array, new_length * sizeof(char));
  if(new_char_array) {
    printf("Realloc successful");
    new_char_array[new_length - 1] = char_to_append;
  }
  else {
    printf("Realloc unsuccessful");
  }
}

// Producing substring from a string.
String string_substr(String target_string, int start_idx, int end_idx) {
  int idx;
  int substrIdx = 0;
  char* substring = malloc((end_idx - start_idx) * sizeof(char));

  for(idx = start_idx; idx < end_idx; idx++, substrIdx++) {
    substring[substrIdx] = target_string.char_array[idx]; 
  }

  String new_substring = String_new(substring);
  return new_substring;
}

int string_is_equal(const String self, const String compared_string) {
  if(self.length == compared_string.length &&
     strcmp(self.char_array, compared_string.char_array) == 0)
    return 1;
  return 0;
}


//
// Vector functions
//

// Function pointer functions.
// VectorString: Add a string to end of VectorString.
void _vector_string_push_back(VectorString* self, const String string_to_append) {
  String* new_string_array = self->string_array;
  int new_length = self->length + 1;
  new_string_array = realloc(new_string_array, new_length * sizeof(String));

  if(new_string_array) {
    printf("Realloc successful");
    new_string_array[new_length - 1] = string_to_append;
  }
  else {
    printf("Realloc unsuccessful");
  }
}

// VectorString: Return index where a string is found.
int _vector_string_find(VectorString* self, const String string_to_find) {
  int idx;

  for(idx = 0; idx < self->length; idx++) {
    if(string_is_equal(self->string_array[idx], string_to_find)) {
      return idx;
    }
  }
  return VECTOR_NOT_FOUND;
}

// VectorString constructor
VectorString VectorString_new(String* string_array, int length) {
  VectorString vector_string;

  vector_string.string_array = string_array;
  vector_string.length = length;
  vector_string.push_back = _vector_string_push_back;
  vector_string.find = _vector_string_find;
  return vector_string;
}

// Must free strings first before we free the vector.
void VectorString_free(VectorString* target_vector) {
  int idx;

  for(idx = 0; idx < target_vector->length; idx++) {
    String temp_string = target_vector->string_array[idx];
    String_free(&temp_string);
  }

  free(target_vector->string_array);
}