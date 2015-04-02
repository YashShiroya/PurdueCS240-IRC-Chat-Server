
//
// Implementation of a HashTable that stores void *
//
#include "HashTableVoid.h"

// Obtain the hash code of a key
int HashTableVoid::hash(const char * key)
{
	// Add implementation here
	int h = 0;
	const char * p = key;
	while (*p) {
		h += *p;
		p++;
	}
	return h % TableSize;
}

// Constructor for hash table. Initializes hash table
HashTableVoid::HashTableVoid()
{
	// Add implementation here
	_buckets = (HashTableVoidEntry**)malloc(TableSize * sizeof (HashTableVoidEntry*));
	for (int i = 0;	i < TableSize; i++) {
		_buckets[i] = NULL;
	}
}

// Add a record to the hash table. Returns true if key already exists.
// Substitute content if key already exists.
bool HashTableVoid::insertItem( const char * key, void * data)
{
	// Add implementation here
		
	int h = hash(key);
	HashTableVoidEntry * e = _buckets[h];
	
	while (e!=NULL) {
		if (!strcmp(e->_key, key)) {
			// Entry found
			e->_data = data;
			return true;
		}
		e = e->_next;
	}
	//Add new
	
	e = new HashTableVoidEntry;
	e->_key = strdup(key);
	e->_data = data;
	e->_next = _buckets[h];
	_buckets[h] = e;

	return false;
}

// Find a key in the dictionary and place in "data" the corresponding record
// Returns false if key is does not exist
bool HashTableVoid::find( const char * key, void ** data)
{
	// Add implementation here
	int h = hash(key);
	HashTableVoidEntry * he = _buckets[h];
	while(he != NULL) {
		if(strcmp(he->_key, key) != 0) {
			he -> _data  = data;
			return true;
		} 
		he = he -> _next;
	}	

	return false;
}

// Removes an element in the hash table. Return false if key does not exist.
bool HashTableVoid::removeElement(const char * key)
{
	// Add implementation here
	return false;
}

// Creates an iterator object for this hash table
HashTableVoidIterator::HashTableVoidIterator(HashTableVoid * hashTable)
{
	// Add implementation here

}

// Returns true if there is a next element. Stores data value in data.
bool HashTableVoidIterator::next(const char * & key, void * & data)
{
	// Add implementation here
	return false;
}

