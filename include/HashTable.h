// Simple hash table template.

#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <cstring> // for strlen
#include <assert.h>
#include <algorithm> // for std::sort
#include <vector>
using std::sort;
#ifdef LOCAL
   #include "../../../AS/include/AS/algorithms.h" // for APESEARCH::swap
#else
   #include <utility> // testing std::swap
#endif

#include <stdlib.h>
#include <time.h>


#define DEFAULTSIZE 4096
#define MAX 16430 // Golden Random number credited by MC
#define LOWEREPSILON -0.175

static inline size_t computeTwosPowCeiling( ssize_t num ) 
   {
   num--; // Account for num already being a two's power
   size_t powerNum = 1;
   for (; num > 0; num >>=1 )
      powerNum <<= 1;
   return powerNum;
   }

static inline size_t computeTwosPow( ssize_t num, bool computeCeiling = true ) 
   {
   num-= computeCeiling; // Account for num already being a two's power
   size_t powerNum = 1;
   ssize_t bound = ( ssize_t )!computeCeiling;
   for (; num > bound; num >>=1 )
      powerNum <<= 1;
   return powerNum;
   }

class FNV
{
public:
   FNV() = default;
   virtual ~FNV() {}
   virtual size_t operator()( const char *data ) const
      {
      //TODO optimize 
      size_t length = strlen(data);

      static const size_t FnvOffsetBasis=146959810393466560;
      static const size_t FnvPrime=1099511628211ul;
      size_t hash = FnvOffsetBasis;
      for (  size_t i = 0; i < length; ++i)
         {
         hash *= FnvPrime;
         hash ^= (unsigned long)data[ i ];
         } // end for
      return hash;
      } //end operator()
};

class Murmur
{
public:
   Murmur() = default;
   virtual ~Murmur(){}
   static inline uint32_t murmur_32_scramble(uint32_t k) 
   {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
   }

   virtual size_t operator()( const char *key ) const
   {
   size_t len = strlen( key );
   uint32_t k;
   uint32_t h = 420;
   
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
   h ^= len;
   h ^= h >> 16;
   h *= 0x85ebca6b;
   h ^= h >> 13;
   h *= 0xc2b2ae35;
   h ^= h >> 16;
   return h;
   }
};

// Compare C-strings, return true if they are the same.
bool CompareEqual( const char *L, const char *R );

class CStringComparator
{
public:

   bool operator()( const char *L, const char *R ) const
      {
      return !strcmp( L, R );
      }

};


template< typename Key, typename Value > class Tuple
   {
   public:
      Key key;
      Value value;

      Tuple( const Key &k, const Value v ) : key( k ), value( v )
         {
         }
   };


template< typename Key, typename Value > class Bucket
   {
   public:
      Bucket *next;
      uint32_t hashValue;
      Tuple< Key, Value > tuple;

      Bucket( const Key &k, const Value v, const uint32_t h ) :
            next( nullptr ) , hashValue( h ), tuple( k, v ) 
         {
         }
      ~Bucket()
         {
         //Recursive delete on bucket-chain
         delete next;
         } // end ~Bucket()
  };

template< typename Key, typename Value>
class PerfectHashing : public FNV
{

   size_t tableSize; // where things can be put 
   std::vector<ssize_t> arr; // equal to tableSize...

  size_t operator()( const char *data, size_t num = 0 ) const 
      {
      size_t length = strlen(data);

      static const size_t FnvOffsetBasis=146959810393466560;
      static const size_t FnvPrime=1099511628211ul;
      if ( !num ) { num = FnvPrime; }
      size_t hash = FnvOffsetBasis;
      for ( size_t i = 0; i < length; ++i )
         {
         hash *= num;
         hash ^= (unsigned long)data[ i ];
         } // end for
      return hash;
      } // end operator()()

public:
   PerfectHashing( size_t _tableSize ) : tableSize( _tableSize ), arr( tableSize )
      {
      }

   ~PerfectHashing() override
      {
      }

   size_t operator()( const char *data ) const override
      {
      ssize_t val = arr[ operator()(data, 0) & ( tableSize - 1 ) ];

      if ( val < 0 )
         return size_t ( ( -val ) - 1 ); // Uses the negative to map to bucket
      
      return operator()( data, ( size_t )val );
      } //end operator()
   
   void removePtrs( std::vector< Bucket< Key, Value> **> &bucketsPlaced )
      {
      for ( Bucket< Key, Value> **bucket : bucketsPlaced )
         *bucket = nullptr;
      }

   Bucket< Key, Value > ** buildInterTable( std::vector< std::vector< Bucket< Key, Value> *> >& buckets, size_t tbSize, size_t numOfBuckets )
      {
      size_t bucketsCnt = 0; assert( tableSize == tbSize );
      tableSize = tbSize;
      arr.resize( tbSize );
      Bucket< Key, Value > **tableArray = new Bucket< Key, Value > *[ tbSize ]();

      std::sort( buckets.begin(), buckets.end(), []( std::vector< Bucket< Key, Value> *>& lhs, 
         std::vector< Bucket< Key, Value> *>& rhs ) { return lhs.size() > rhs.size(); } );
      
      // First place all buckets with collisions into seperate buckets
      typename std::vector< std::vector< Bucket< Key, Value> *> >::iterator bucketVec = mapCollisionsToUniqEntry( buckets, tableArray, bucketsCnt );

      // At this point bucketVec should only point to vectors of size one...
      // Now just map the single buckets into unique places
      if ( bucketVec != buckets.end() )
         mapSingleBuckets( buckets, bucketVec, tableArray, bucketsCnt );

      assert( numOfBuckets == bucketsCnt );
      return tableArray;
      }
      
   typename std::vector< std::vector< Bucket< Key, Value> *> >::iterator mapCollisionsToUniqEntry( std::vector< std::vector< Bucket< Key, Value> *> >& buckets, 
      Bucket< Key, Value > ** const tableArray, size_t& bucketsCnt )
      {
      typename std::vector< std::vector< Bucket< Key, Value> *> >::iterator bucketVec = buckets.begin();
      std::vector< Bucket< Key, Value> **> bucketsPlaced; // To keep track of where buckets are part since might need to retry...
      for ( ;bucketVec != buckets.end() && bucketVec->size() > 1; ++bucketVec, bucketsPlaced.clear() )
         {
         ssize_t arg = 1;
         size_t tempOf = 0;

         // Try hashing with arg until every bucket inside bucketVec maps to a unique top-level bucket
         typename std::vector< Bucket< Key, Value> *>::iterator bucket = bucketVec->begin();
         while ( bucket != bucketVec->end() )
            {
            //std::vector< Bucket< Key, Value> *>& vec = *bucketVec;
            size_t ind = operator()( ( *bucket )->tuple.key, static_cast<size_t>( arg ) ) & ( tableSize - 1 );
            Bucket< Key, Value > ** mainLevel = tableArray + ind;
            assert( mainLevel < tableArray + tableSize );
            if ( *mainLevel ) // Collision so need to retry
               {
               removePtrs( bucketsPlaced );
               bucketsPlaced.clear();
               ++arg;
               tempOf = 0;
               bucket = bucketVec->begin();
               } // end if
            else // empty bucket
               {
               ( *bucket )->next = nullptr;
               *mainLevel = *bucket;
               bucketsPlaced.emplace_back( mainLevel );
               ++tempOf;
               ++bucket;
               }
            } // end while 
         assert( tempOf == bucketsPlaced.size() );
         size_t bucketVecSize = bucketVec->size();
         assert( tempOf == bucketVecSize );
         bucketsCnt += tempOf;

         // Update the intermediate array
         size_t hash = operator()( bucketVec->front()->tuple.key, 0 );
         size_t hashInd = hash & ( tableSize - 1 );
         arr[ hashInd ] = arg;
         } // end for

         return bucketVec;
      } // end mapCollisionsToUniqEntry()
   
   void mapSingleBuckets( std::vector< std::vector< Bucket< Key, Value> *> >& buckets, 
      typename std::vector< std::vector< Bucket< Key, Value> *> >::iterator bucketVec, 
         Bucket< Key, Value > ** const tableArray, size_t& bucketsCnt )
      {
      assertSingleVectInvariants( bucketVec, buckets.end() );
      // Find all empty buckets now
      size_t bucketsRemaining = static_cast<size_t>( buckets.end() - bucketVec );
      std::vector< Bucket< Key, Value> **> freeBuckets;
      freeBuckets.reserve( bucketsRemaining );
      for ( Bucket< Key, Value > **mainLevel = tableArray, **end = tableArray + tableSize; tableArray != end && bucketsRemaining ; ++mainLevel )
         {
         if ( !( *mainLevel ) )
            {
            freeBuckets.emplace_back( mainLevel );
            --bucketsRemaining;
            }
         } // end for
      assert( !bucketsRemaining );
      assert( freeBuckets.size() == size_t( buckets.end() - bucketVec ) );
      // Place all single buckets now into these empty slots
      for ( ;bucketVec != buckets.end(); ++bucketVec, freeBuckets.pop_back() )
         {
         bucketVec->front()->next = nullptr;
         *freeBuckets.back() = bucketVec->front();
         ssize_t arrayInd = freeBuckets.back() - tableArray;
         arrayInd = ( -arrayInd ) - 1;
         size_t hash = operator()( bucketVec->front()->tuple.key, 0 );
         size_t hashInd = hash & ( tableSize - 1 );
         arr[ hashInd ] = arrayInd;
         ++bucketsCnt;
         } // end for
      } // end placeSingleBucketsInEmptyBuckets()

   void assertSingleVectInvariants( typename std::vector< std::vector< Bucket< Key, Value> *> >::iterator currBucket, 
      const typename std::vector< std::vector< Bucket< Key, Value> *> >::iterator end )
      {
      for ( ; currBucket != end; ++currBucket )
         assert( currBucket->size() == 1 );
      }

};


template< typename Key, typename Value, class Hash = Murmur, class Comparator = CStringComparator > class HashTable
   {
   private:

      // Your code here.

      size_t tableSize; // length of bucket array
      Bucket< Key, Value > **buckets;
      size_t numberOfBuckets; // Contains amount of seperate chained buckets
      size_t collisions = 0; // Tracks current collisions in hash_table
      Comparator compare;
      Hash *hashFunc;

      template< Key, Value>
      friend class Iterator;
      friend class HashBlob;
   
   std::vector< Bucket< Key, Value> *> flattenHashTable()
      {
      
      Bucket< Key, Value > *currBucket = *buckets;
      Bucket< Key, Value > **mainLevel = buckets;
      std::vector< Bucket< Key, Value> * > bucketVec;
      bucketVec.reserve( numberOfBuckets );

      // ++ happens first then dereference
      for ( Bucket< Key, Value > **const end = buckets + tableSize; 
            mainLevel != end && bucketVec.size() < numberOfBuckets; ++mainLevel )
         {
         for ( currBucket = *mainLevel ; currBucket ; currBucket = currBucket->next )
            bucketVec.emplace_back( currBucket );
         }

      return bucketVec;
      }  // end flattenHashTable()


   Bucket< Key, Value > **helperFind( const Key& k, uint32_t hashVal ) const
      {
      // Applying a bit-wise mask on the least-sig bits
      size_t index = (size_t)hashVal & ( tableSize - 1 ); // modulo operation using bitwise AND (only works with power of two table size)
      Bucket< Key, Value > **bucket = buckets + index;

      for ( ; *bucket; bucket = &( *bucket )->next )
         {
         if ( compare( ( *bucket )->tuple.key, k ) )
            break;
         }
      return bucket;
      } // end helperFind()

   public:

      Tuple< Key, Value > *Find( const Key& k, const Value initialValue )
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) entry.  If the key is not already
         // in the hash, add it with the initial value.

         // Your code here
         uint32_t hashVal = static_cast<uint32_t>( (*hashFunc)( k ) );
         Bucket< Key, Value > **bucket = helperFind( k, hashVal );
         
         // Checks for nullptr
         if( !*bucket )
            {
            if ( buckets[ hashVal & ( tableSize - 1 ) ] )
               ++collisions;
            *bucket = new Bucket< Key, Value >( k, initialValue, hashVal );
            ++numberOfBuckets;
            } // end if
      
         return & ( * bucket )->tuple;
         }
      size_t getCollisions() const { return collisions; }

      Tuple< Key, Value > *Find( const Key& k ) const
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) enty.  If the key is not already
         // in the hash, return nullptr.

         // Your code here.
         Bucket< Key, Value > *bucket = *helperFind( k, ( uint32_t )(*hashFunc)( k ) );

         // If not nullptr, entry was found so returning reference to tuple...
         return bucket ? &bucket->tuple : nullptr;
         } // end Find()


      //!Invalidates any Iterators / pointers
      // Modify or rebuild the hash table as you see fit
      // to improve its performance now that you know
      // nothing more is to be added.
      void Optimize( double loadFactor = 0.5, bool computeCeiling = true ) // does this imply load factor reaching this point?
         {
         // It might be the case that the bucket size is far lower than expected
         // So it might be necessary to shrink the table size
         size_t expectedTS = size_t ( static_cast<double>(numberOfBuckets) / loadFactor );

         std::vector< Bucket< Key, Value > *> flattened = flattenHashTable();
         delete []buckets;
         
         // Sort so that most frequent words are inserted first
         std::sort( flattened.begin(), flattened.end(), 
            []( Bucket< Key, Value > *lhs, Bucket< Key, Value > *rhs ) 
               { return lhs->tuple.value > rhs->tuple.value; } );

         // Doubles number of buckets and computes the two's power ceiling
         // .e.g computeTwosPow( 100 * 2 ) = 256
         size_t newTbSize = computeTwosPow( (ssize_t) (expectedTS - 1 ), computeCeiling );
         // Check if load factor is too small
         //if ( (static_cast<double>(numberOfBuckets) / static_cast<double>( newTbSize ) ) - loadFactor < LOWEREPSILON )
         //   newTbSize >>= 1; // Instead strink table size by a power of two
         //size_t newTbSize = computeTwosPow( (ssize_t) (expectedTS - 1), computeCeiling );

         // Adjust member variables
         buckets = new Bucket< Key, Value> *[ newTbSize ];
         memset( buckets, 0, sizeof(Bucket< Key, Value > *) * newTbSize );
         tableSize = newTbSize;
         collisions = 0;

         // Insert each bucket back into the table
         for ( Bucket< Key, Value > *bucket : flattened )
            {
            bucket->next = nullptr; // Remove any pointer relationship
            size_t index = (size_t)bucket->hashValue & ( tableSize - 1 );
            Bucket< Key, Value > **bucketPtr = buckets + index;
            if ( *bucketPtr )
               ++collisions;
            // Goes all the way to the end of linked list
            for ( ; *bucketPtr; bucketPtr = &( *bucketPtr )->next ); //iterates until end of linked list chain
            *bucketPtr = bucket;
            } // end for
         }
      // Used for optimizing to minimal perfect hash function
      void Optimize( double load_factor/* = 0.407 */)
         {
         Optimize( load_factor, false );
         
         if ( !collisions )
            return;
         // Now do perfect hashing
         std::vector< std::vector< Bucket< Key, Value> *> > bucketsVec = vectorOfBuckets();
         assert( bucketsVec.size() == numOfLinkedLists() );
         delete hashFunc;
         delete []buckets;
         hashFunc = new PerfectHashing<Key, Value>( tableSize );
         buckets = dynamic_cast<PerfectHashing<Key, Value>*>(hashFunc)->buildInterTable( bucketsVec, tableSize, numberOfBuckets );
         size_t numOfLL = numOfLinkedLists();
         collisions = numberOfBuckets - numOfLL;
         assert( collisions == 0 );
         }
      


      // Your constructor may take as many default arguments
      // as you like.

      HashTable( size_t tb = DEFAULTSIZE, Hash *hasher = new Murmur(), Comparator comp = CStringComparator() ) : tableSize( computeTwosPow( (ssize_t)tb ) ), 
         buckets( new Bucket< Key, Value > *[ tableSize ] ), numberOfBuckets( 0 ), compare( comp ), hashFunc( hasher )
         {
         assert( tb );
         // Your code here.
         memset( buckets, 0, sizeof(Bucket< Key, Value > *) * tb );
         }

      ~HashTable( )
         {
         // Your code here.
         for ( Bucket< Key, Value > **bucket = buckets, 
               ** const end = buckets + tableSize; bucket != end; ++bucket )
            delete *bucket;

         delete hashFunc;
         delete[] buckets;             
         } // end ~HashTable()


      std::vector< const Bucket< Key, Value> * > constflattenHashTable()
         {
         Bucket< Key, Value > const *currBucket = *buckets;
         Bucket< Key, Value > **mainLevel = buckets;
         std::vector< const Bucket< Key, Value> * > bucketVec;
         bucketVec.reserve( numberOfBuckets );

         for ( Bucket< Key, Value > **const end = buckets + tableSize; 
               mainLevel != end && bucketVec.size() < numberOfBuckets; ++mainLevel )
            {
            for ( currBucket = *mainLevel ; currBucket ; currBucket = currBucket->next )
               bucketVec.emplace_back( currBucket );
            }  

         return bucketVec;
         }
      
      void swap( HashTable& other )
         {
         #ifdef LOCAL
            using APESEARCH::swap;
         #else
            using std::swap;
         #endif
         swap( buckets, other.buckets );
         swap( tableSize, other.tableSize );
         swap( numberOfBuckets, other.numberOfBuckets );
         swap( collisions, other.collisions );
         swap( hashFunc, other.hashFunc );
         swap( compare, other.compare );
         } 

      inline size_t size() const { return numberOfBuckets; }
      inline size_t table_size() const { return tableSize; }
      inline double load_factor() const { return static_cast<double>( numberOfBuckets ) / tableSize; }
      inline double ratioOfColli() const { return static_cast<double>( collisions ) / numberOfBuckets; }

      inline size_t numOfLinkedLists() const
         {
         size_t numOfLL = 0;
         for ( Bucket< Key, Value > **mainLevel = buckets, **end = buckets + tableSize; mainLevel != end; ++mainLevel )
            if ( *mainLevel )
               ++numOfLL;
         return numOfLL;
         }
      
      bool advancePtr( Bucket< Key, Value > ***mainLevel, Bucket< Key, Value > **end, std::vector< std::vector< Bucket< Key, Value> *> >& bucketVec ) const
         {
         for ( ; *mainLevel != end; ++(*mainLevel) )
            {
            if ( **mainLevel )
               {
               bucketVec.emplace_back( std::vector< Bucket< Key, Value> *>() );
               return true;
               }
            } // end if
         return false;
         }
      
private:

      void assertLinkedList( std::vector< Bucket< Key, Value> *>& vec ) const
         {
         for ( typename std::vector< Bucket< Key, Value> *>::iterator currBucket = vec.begin(); currBucket != vec.end(); ++currBucket )
            {
            if ( currBucket + 1 == vec.end() )
               assert( !( ( *currBucket )->next ) );
            else 
               assert( ( *currBucket )->next == *( currBucket + 1 ) );
            }
         } // end assertLinkedList()
      std::vector< std::vector< Bucket< Key, Value> *> > vectorOfBuckets() const
         {
         Bucket< Key, Value > **mainLevel = buckets;
         Bucket< Key, Value > **const end = buckets + tableSize;
         std::vector< std::vector< Bucket< Key, Value> *> > bucketVec;
         bucketVec.reserve( numOfLinkedLists() );
         size_t bucketCnt = 0;
         for ( ;advancePtr( &mainLevel, end, bucketVec ); ++mainLevel )
            {
            std::vector< Bucket< Key, Value> *>& bucketRef = bucketVec.back();
            assert( !bucketRef.size() );
            for ( Bucket< Key, Value > *currBucket = *mainLevel ; currBucket ; currBucket = currBucket->next )
               {
               bucketRef.emplace_back( currBucket );
               ++bucketCnt;
               }
            assert( bucketRef.size() );
            bucketRef.shrink_to_fit();
            assertLinkedList( bucketRef );
            } // end for
         assert( bucketCnt == numberOfBuckets );
         return bucketVec;
         } // end vectorOfBuckets()
public:
      double averageCollisonsPerBucket() const
         {
         if ( !numberOfBuckets )
            return 0;
         size_t numOfLL = numOfLinkedLists();
         return static_cast<double> ( numberOfBuckets - numOfLL ) / static_cast<double> ( numOfLL );
         }

      double averageBucketsPerLL() const 
         {
         if ( !numberOfBuckets )
            return 0;
         size_t numOfLL = numOfLinkedLists();
         return static_cast<double> ( numberOfBuckets ) / static_cast<double> ( numOfLL );
         }

      void printStats() const 
         {
         int width = 33;
         std::cout << "------- Hash Table Stats Beg -------\n";
         std::cout << std::setw( width ) << std::left << "Total Buckets Allocated: " << size() << '\n';
         std::cout << std::setw( width ) << std::left << "Table Size: " << table_size() << '\n';
         std::cout << std::setw( width ) << std::left << "load_factor: " << load_factor() << '\n';
         std::cout << std::setw( width ) << std::left << "Percentage of Collisions: " << ratioOfColli() << '\n';
         std::cout << std::setw( width ) << std::left << "Average Collisions per Bucket: "  << averageCollisonsPerBucket() << '\n';
         std::cout << std::setw( width ) << std::left << "Average Buckets Per Linked List: " << averageBucketsPerLL() << '\n';
         std::cout << "------- Hash Table Stats End -------\n";
         }

      template<typename KeyItr = Key, typename ValueItr = Value>
      class Iterator
         {
         private:

            friend class HashTable;
            // Your code here.
            HashTable const *table; // for the tableSize
            Bucket< Key, Value > **mainLevel;
            Bucket< Key, Value > **currentBucket;

            Iterator( HashTable const *_table, size_t bucket ) :  table( _table ), mainLevel( table->buckets + bucket ) {
               for (  Bucket< Key, Value > **end = table->buckets + table->tableSize
                  ; mainLevel != end && !(*mainLevel); ++mainLevel ); // Order of comparison is required: *mainLevel could be an invalid read if mainLevel == end
               currentBucket = mainLevel;
            }

            // This constructor can be used for finds that want to return an iterator instead
            Iterator( HashTable const *_table, Bucket<Key, Value> **b ) :  table( _table ), 
                  mainLevel( table ? table->buckets + ( ( *b )->hashValue  & ( table->tableSize - 1 ) ) : nullptr ), currentBucket( b ) {}

            Iterator( HashTable *_table, size_t bucketInd, Bucket<Key, Value> *b ) :  table( _table ), mainLevel( table ? table->buckets + bucketInd : nullptr ), currentBucket( mainLevel )
               {
               // Your code here.
               if ( !table )
                  return;
               assert( bucketInd < table->tableSize );

               // Follows linked list until either reaches something or a nullptr
               for( ; *currentBucket && *currentBucket != b; currentBucket = & ( *currentBucket )->next );

               if ( ! ( *currentBucket ) )
                  currentBucket = mainLevel = table->buckets + table->tableSize;
               }

            inline void advanceBucket( )
               {
               if ( ( *currentBucket )->next )
                  currentBucket = & ( *currentBucket )->next;
               else
                  {
                  ++mainLevel; // Go to next place
                  // Order of comparasions matter since *(buckets + tableSize) isn't owned by us 
                  // Need to find a top-level bucket that's valid
                  for ( Bucket< Key, Value > **end = table->buckets + table->tableSize
                     ; mainLevel != end && !(*mainLevel); ++mainLevel ); 
                  currentBucket = mainLevel;
                  }
               } // end advanceBucket()

         public:

            Iterator( ) : Iterator( nullptr, 0, nullptr )
               {
               }

            ~Iterator( )
               {
               }

            Tuple< Key, Value > &operator*( )
               {
               // Your code here.
               return ( *currentBucket )->tuple;
               }

            Tuple< Key, Value > *operator->( ) const
               {
               // Your code here.
               return & ( *currentBucket )->tuple;
               }

            // Prefix ++
            Iterator &operator++( )
               {
               advanceBucket();
               return *this;
               }

            // Postfix ++
            Iterator operator++( int )
               {
               Iterator old( *this );
               advanceBucket();
               return old;
               }

            bool operator==( const Iterator &rhs ) const
               {
               return currentBucket == rhs.currentBucket;
               }

            bool operator!=( const Iterator &rhs ) const
               {
               return currentBucket != rhs.currentBucket;
               }

            Iterator operator+( ssize_t var )
               {
               Iterator copy( *this );
               for ( ssize_t n = 0; n < var; ++n )
                  copy.advanceBucket();
               return copy;
               }
         };
      
      typedef Iterator<Key, Value> iterator;
      typedef Iterator<const Key, const Value> const_iterator;

      iterator begin( ) const
         {
         return iterator( this, size_t ( 0 ) );
         }

      iterator end( ) const
         {
         return iterator( this, tableSize );
         }

      const_iterator cbegin( ) const
         {
          return const_iterator( this, size_t ( 0 ) );
         }
      
      const_iterator cend( ) const
         {
         return const_iterator( this, tableSize );
         }
      
      iterator FindItr( const Key& k ) const
         {
         Bucket< Key, Value > **bucket = helperFind( k, ( uint32_t )( *hashFunc )( k ) );

         return *bucket ? iterator( this, bucket ) : end();
         }
      
      //! Warning while this is simple, it also isn't very efficient
      void OptimizeElegant( double loadFactor = 0.5 )
         {
         size_t expectedTS = size_t ( static_cast<double>(numberOfBuckets) / loadFactor );
         size_t newTbSize = computeTwosPow( (ssize_t) expectedTS );

         HashTable temp ( newTbSize );

         for ( Iterator<Key, Value> itr = begin(); itr != end(); ++itr )
            {
            Tuple <Key, Value> *pair = &( *itr.currentBucket )->tuple;
            temp.Find( pair->key, pair->value );
            }
         swap( temp );
         }
   
   };


   template<typename Key, typename Value >
   void swap( HashTable< Key, Value>& lhs, HashTable< Key, Value>& rhs )
      {
      lhs.swap( rhs );
      }

