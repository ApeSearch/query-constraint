#pragma once

// HashBlob, a serialization of a HashTable into one contiguous
// block of memory, possibly memory-mapped to a HashFile.

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <exception>
#include <cerrno> // for errno
#ifdef MACOS
   #include <malloc/malloc.h>
#endif
#ifdef LINUX
   #include <malloc.h>
#endif
#include <unistd.h>
#include <sys/mman.h> // for mmap and munmap

#include "HashTable.h" // for hashTable
#include <utility> // for std::swap and std::move

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-length-array" // You can trust us

using Hash = HashTable< const char *, size_t >;
using Pair = Tuple< const char *, size_t >;
using HashBucket = Bucket< const char *, size_t >;


static const size_t Unknown = 0;

constexpr size_t RoundUpConstExpr( size_t length, size_t boundary = 8 )
   {
   return ( length + ( boundary - 1 ) ) & ~( boundary - 1 );
   }

size_t RoundUp( size_t length, size_t boundary = 8 )
   {
   // Round up to the next multiple of the boundary, which
   // must be a power of 2.

   static const size_t oneless = boundary - 1,
      mask = ~( oneless );
   return ( length + oneless ) & mask;
   }


struct SerialTuple
   {
   // This is a serialization of a HashTable< char *, size_t >::Bucket.
   // One is packed up against the next in a HashBlob.

   // Since this struct includes size_t and uint32_t members, we'll
   // require that it be sizeof( size_t ) aligned to avoid unaligned
   // accesses.

   public:

      // SerialTupleLength = 0 is a sentinel indicating
      // this is the last SerialTuple chained in this list.
      // (Actual length is not given but not needed.)

      size_t Length, Value;
      uint32_t HashValue;

      static constexpr size_t sizeOfMetaData = sizeof( size_t ) * 2 + sizeof( uint32_t ); // About 20
      static constexpr size_t sizeOfNullSentinel = sizeof( size_t ); // Just needs to be a Length

      // The Key will be a C-string of whatever length.
      char Key[ Unknown ];

      // Calculate the bytes required to encode a HashBucket as a
      // SerialTuple.
      static size_t helperBytesRequired( const Pair& pair )
         {
         // Extra character for null-character
         size_t keyLen = strlen( pair.key ) + 1;

         return RoundUp( SerialTuple::sizeOfMetaData + keyLen, 0x8 );
         }

      static size_t BytesRequired( const HashBucket *b )
         {
         return helperBytesRequired( b->tuple );
         }

      // Write the HashBucket out as a SerialTuple in the buffer,
      // returning a pointer to one past the last character written.

      static SerialTuple *initSerialTuple( char *buffer, const HashBucket *b, size_t lengthSerialized )
         {
         SerialTuple *serialTuple = reinterpret_cast< SerialTuple * >( buffer );

         serialTuple->Length = lengthSerialized;
         serialTuple->Value = b->tuple.value;
         serialTuple->HashValue = b->hashValue;

         return serialTuple;
         }

      static char *Write( char *buffer, char *bufferEnd,
            const HashBucket *b )
         {
         // Your code here.
         size_t bytesReq = BytesRequired( b );

         assert( static_cast<size_t> ( bufferEnd - buffer ) >= bytesReq );

         SerialTuple *serialTuple = initSerialTuple( buffer, b, bytesReq );

         char *key = serialTuple->Key;

         size_t charToWrite = bytesReq - SerialTuple::sizeOfMetaData;
         assert( key + charToWrite <= bufferEnd );

         assert( strncpy( key, b->tuple.key, charToWrite ) == key );

         return buffer + bytesReq;
         } // end Write()
      
      static char *WriteNull( char *buffer, char *bufferEnd )
         {
         // Ensures that the buffer can fit the null sentinel to begin with...
         assert( size_t( bufferEnd - buffer ) >= SerialTuple::sizeOfNullSentinel );

         SerialTuple *nullSerial = reinterpret_cast< SerialTuple * >( buffer ); 
         nullSerial->Length = 0;

         return buffer + SerialTuple::sizeOfNullSentinel;
         // not buffer + SerialTuple::sizeOfMetaData since memory is not aligned 8-byte
         }
  };


class HashBlob
   {
   // This will be a hash specifically designed to hold an
   // entire hash table as a single contiguous blob of bytes.
   // Pointers are disallowed so that the blob can be
   // relocated to anywhere in memory

   // The basic structure should consist of some header
   // information including the number of buckets and other
   // details followed by a concatenated list of all the
   // individual lists of tuples within each bucket.
   friend class Const_Iterator;

   public:
      static constexpr size_t decidedMagicNum = 69;
      static constexpr size_t version = 1;

      // Define a MagicNumber and Version so you can validate
      // a HashBlob really is one of your HashBlobs.

      size_t MagicNumber,
         Version,
         BlobSize,
         NumberOfBuckets,
         Buckets[ Unknown ];
      // The SerialTuples will follow immediately after.


      // Search for the key k and return a pointer to the
      // ( key, value ) entry.  If the key is not found,
      // return nullptr.
      template<class HashFunc = Murmur> 
      const SerialTuple *Find( const char *key ) const
         {
         static HashFunc func;

         uint8_t const *byteAddr = reinterpret_cast< uint8_t const * > ( &MagicNumber );

         uint32_t hashVal = ( uint32_t )func( key );
         size_t bucketInd = hashVal & ( NumberOfBuckets - 1 );
         size_t offset = Buckets[ bucketInd ];

         // If non-zero, we have found a list to look into
         if ( offset )
            {
            // Okay since everything is continous in memory...
            byteAddr += offset;
            SerialTuple const *tupleArr = reinterpret_cast<SerialTuple const *>( byteAddr );

            while ( tupleArr->Length )
               {
               if ( CompareEqual( tupleArr->Key, key ) )
                  return tupleArr;
               byteAddr += tupleArr->Length;
               tupleArr = reinterpret_cast<SerialTuple const *>( byteAddr );
               } // end if
            }
         return nullptr;
         }
      
      bool verifyHashBlob() const
         {
         return MagicNumber == HashBlob::decidedMagicNum &&
               Version == HashBlob::version;
         }

      inline static size_t BytesForHeaderBuckets( const Hash *hashTable )
         {
         size_t sizeOfBuckets = sizeof( size_t ) * hashTable->tableSize;
         size_t header = sizeof( HashBlob );
         return sizeOfBuckets + header;
         }

      // Calculate how much space it will take to
      // represent a HashTable as a HashBlob.

      // Need space for the header + buckets +
      // all the serialized tuples.
      static size_t BytesRequired( const Hash *hashTable )
         {
         size_t totSizeOfSerialTuples = 0; // Count bytes that go to serial tuples...

         // Add up the bytes needed for every bucket
         for ( Hash::const_iterator itr = hashTable->cbegin(); itr != hashTable->cend(); ++itr )
            totSizeOfSerialTuples += SerialTuple::helperBytesRequired( *itr );

         // Add null sentinal bytes
         totSizeOfSerialTuples += hashTable->numOfLinkedLists() * SerialTuple::sizeOfNullSentinel;

         // And finaly added to the bytes needed for the header + buckets array
         size_t totBytes = totSizeOfSerialTuples + BytesForHeaderBuckets( hashTable );
         // It should be the case that the resultant value is a multiple of 8
         assert( RoundUp( totBytes, 8 ) == totBytes );
         return totBytes;
         }

      // Write a HashBlob into a buffer, returning a
      // pointer to the blob.

      static HashBlob *Write( HashBlob *hb, size_t bytes,
            const Hash *hashTable )
         {
         // placeholders for now
         hb->MagicNumber = HashBlob::decidedMagicNum;
         hb->Version = HashBlob::version;

         
         // Initialize the rest of variables
         hb->BlobSize = bytes;
         hb->NumberOfBuckets = hashTable->table_size();

         // Points to the beginning of the Serial Tuples...
         char *serialPtr =reinterpret_cast< char *>( hb->Buckets + hashTable->table_size() );
         char *end = reinterpret_cast< char *>( hb ) + bytes;

         std::vector< std::vector< HashBucket *> > buckets( hashTable->vectorOfBuckets() );

         typename std::vector< std::vector< HashBucket *> >::iterator bucketsVec = buckets.begin();

         // Iterate through every bucket and append it to the serial tuples list
         for ( ; bucketsVec != buckets.end(); ++bucketsVec )
            {
            // Write the absolute address into the bucket represented...
            //! Note will not work with minimal perfect hashing
            size_t bucketInd = bucketsVec->front()->hashValue & ( hashTable->table_size() - 1 );
            hb->Buckets[ bucketInd ] = size_t( serialPtr - reinterpret_cast< char * >( hb ) );

            for ( typename std::vector<HashBucket*>::iterator bucket = bucketsVec->begin();
                  bucket != bucketsVec->end(); ++bucket )
               serialPtr = SerialTuple::Write( serialPtr, end, *bucket );
            
            // Now add null serial Tuple
            serialPtr = SerialTuple::WriteNull( serialPtr, end );
            } // end for
         assert( end == serialPtr );
         return hb;
         }

      // Create allocates memory for a HashBlob of required size
      // and then converts the HashTable into a HashBlob.
      // Caller is responsible for discarding when done.

      // (No easy way to override the new operator to create a
      // variable sized object.)
      static HashBlob *Create( const Hash *hashTable )
         {
         const size_t bytesReq = HashBlob::BytesRequired( hashTable );

         // Need to use malloc to get the exact right number of bytes
         char *buffer = ( char * ) malloc( bytesReq );
         memset( buffer, 0, bytesReq );

         return Write( reinterpret_cast<HashBlob *>( buffer ), bytesReq, hashTable );
         }

      // Discard
      static void Discard( HashBlob *blob )
         {
         free( blob );
         }

//------------------------------------------------------------------------------------------------
//
//                                 Const_Iterator Definition
//
//------------------------------------------------------------------------------------------------
/*
 * Used to help with testing hashblobs and the serial tuples. Will iterate through the serial tuples
 * until it reaches the end specified by bufferEnd.
 * 
 * buffer: A byte-addressed pointer that points to the beginning of a serial tuple. Reinterpreted as 
 *         a serial tuple whenever the value needs to be dereferenced.
 * bufferEnd: Points to one past the last byte in the hashblob. Used to represent when an iterator
 *          has reached the end.
*/
      class Const_Iterator
         {
         
         friend class HashBlob;

         HashBlob const *hashBlob;

         char const *buffer;
         char const * const bufferEnd;

         /*
          * REQUIRES: buffer != bufferEnd i.e. buffer doesn't point to the end of the hashblob
          *  MODIFES: buffer ( incremented in a way such that it points to the beginning of the next serial tuple or bufferEnd )
          *  EFFECTS: Uses the serial tuple's length variable to move buffer so that it points to the 
          *           beginning of the next serial tuple. In the case that the next is a null serial,
          *           moves buffer again past the null sentinal.
          * 
         */
         inline void advanceSerialTuple( )
            {
            assert( buffer != bufferEnd );

            SerialTuple const *tuple = reinterpret_cast< SerialTuple const * >( buffer );

            // Needs to point to a valid serial tuple ( not null )
            assert( tuple->Length );

            buffer += tuple->Length;

            tuple = reinterpret_cast< SerialTuple const * >( buffer );

            // check if reached null serial; if so, advance again.
            if ( buffer != bufferEnd && !tuple->Length )
               buffer += SerialTuple::sizeOfNullSentinel;

            } // end advanceSerialTuple()

         Const_Iterator( HashBlob const *hb, const std::size_t offset ) : hashBlob( hb ),
            buffer( hb ? reinterpret_cast< char const * >( hb->Buckets + hb->NumberOfBuckets ) + offset : nullptr ), 
            bufferEnd( hb ? reinterpret_cast< char const * > ( &hb->MagicNumber ) + hb->BlobSize : nullptr ) {}

      public:
         Const_Iterator( ) : hashBlob( nullptr ), buffer( nullptr ), bufferEnd( nullptr ) {}

         ~Const_Iterator( ) {}

         const SerialTuple &operator*( ) const
            {
            return *( reinterpret_cast< SerialTuple const *> ( buffer ) );
            } // end Dereference operator()

         SerialTuple const *operator->( ) const
            {
            return reinterpret_cast< SerialTuple const *>( buffer );
            }

         Const_Iterator &operator++( )
            {
            advanceSerialTuple();
            return *this;
            } // end prefix

         Const_Iterator operator++(int)
            {
            Const_Iterator old( *this );
            advanceSerialTuple();
            return old;
            } // end postfix

         bool operator==( const Const_Iterator & rhs ) const
            {
            return buffer == rhs.buffer;
            }
         bool operator!=( const Const_Iterator & rhs ) const
            {
            return buffer != rhs.buffer;
            }
         
         bool operator==( const char *rhs ) const
            {
            return buffer == rhs;
            }
         bool operator!=( const char *rhs ) const
            {
            return buffer != rhs;
            }

         Const_Iterator operator+( ssize_t var )
            {
            Const_Iterator copy( *this );
            for ( ssize_t n = 0; n < var; ++n )
               copy.advanceSerialTuple();
            return copy;
            } // end operator+()

         const HashBlob *Blob( ) const
            {
            return hashBlob;
            }

         };
      
      Const_Iterator cbegin( ) const
         {
         return Const_Iterator( this, 0 );
         }

      Const_Iterator cend( ) const
         {
         const std::size_t bytesBeforeSerialTupleBeg = (size_t) ( reinterpret_cast< char const * >( Buckets + NumberOfBuckets ) 
            - reinterpret_cast< char const * > ( &MagicNumber ) );
         return Const_Iterator( this, BlobSize - bytesBeforeSerialTupleBeg );
         }

   };

// A RAII for Files
class File
   {
   int fd;
   friend class failure;
public:

   class failure : public std::exception
      {
      friend class File;
   protected:
      int error_number;
      int error_offset;
      std::string error_message;

   public:
      explicit failure( const std::string& msg, int err_num, int err_off )
         : error_number( err_num ), error_offset( err_off ), error_message( msg ) {}

      explicit failure( const std::string&& msg, int err_num, int err_off )
         : error_number( err_num ), error_offset( err_off ), error_message( std::move( msg ) ) {}
      
      virtual ~failure() throw () {}

      virtual const char *what() const throw()
         {
         return error_message.c_str();
         }

      virtual int getErrorNumber() const throw() 
         {
         return error_number;
         }

      virtual int getErrorOffset() const throw()
         {
         return error_offset;
         }
      };

   File( ) : fd( -1 ) {}

   File( const char *pathname, int flags, mode_t mode ) : fd( open( pathname, flags, mode ) )
      {
      if ( fd == -1 )
         {
         perror("Error opening file");
         throw failure( "Issue with opening file", errno, 0 );
         } // end if
      }  // end File

   File( const char *pathname, int flags ) : fd( open( pathname, flags ) )
      {
      if ( fd == -1 )
         {
         perror("Error opening file");
         throw failure( "Issue with opening file", errno, 0 );
         } // end if
      }

   // More efficent than std::swap
   File( File&& file ) : fd( file.fd )
      {
      file.fd = -1;
      }

   File& operator=( File&& file )
      {
      #ifdef LOCAL
            using APESEARCH::swap;
      #else
            using std::swap;
      #endif
      swap( fd, file.fd );
      return *this;
      }

   // Const Operator and constructor or not allowed
   File( const File& ) = delete;
   File& operator=( const File& ) =delete;

   ~File()
      {
      if ( fd != -1 )
         close( fd );
      } // end ~File()

   inline int getFD() const
      {
      return fd;
      }
   };

//------------------------------------------------------------------------------------------------
//
//                                  unique_mmap Definition 
//
//------------------------------------------------------------------------------------------------
/*
 * A RAII for mmap. It ensures that at the end of the scope, an mmapped file is guaranteed to
 * deallocate and free any acquired resources.
 * 
 *         map: Pointer that holds the memory mapped region.
 * bytesMapped: The amount of bytes that was mapped into memory. Mainly used for calling munmap so that
 *              the object knows how many bytes is needed to be freed.
 *       file: A file object. If such an object is passed into the constructor, unique_mmap becomes
 *              responsible for the deallocation ( closing of the file object ).
*/
class unique_mmap
   {
   void *map;
   size_t bytesMapped;
   File file;

public:
   class failure : public std::exception
      {
      friend class unique_mmap;
   protected:
      int error_number;
      int error_offset;
      std::string error_message;

   public:
      explicit failure( const std::string& msg, int err_num, int err_off )
         : error_number( err_num ), error_offset( err_off ), error_message( msg ) {}

      explicit failure( const std::string&& msg, int err_num, int err_off )
         : error_number( err_num ), error_offset( err_off ), error_message( std::move( msg ) ) {}
      
      virtual ~failure() throw () {}

      virtual const char *what() const throw()
         {
         return error_message.c_str();
         }

      virtual int getErrorNumber() const throw() 
         {
         return error_number;
         }

      virtual int getErrorOffset() const throw()
         {
         return error_offset;
         }

      static constexpr int LENGTHZERO = 100; 
      };
   
   unique_mmap() : map ( nullptr ) {}

   /*
    * FYI:
    * addr: asks which address in which one would like to map pointer to.
    *       If NULL, the kernel chooses the address (page aligned ofc)
    *       in which to linked the mmap to.
    *       O.W., the kenrel takes it as a hint and places the mapping in
    *       a page boundary, ( which is alwys within the arena ( linux specifies:
    *       /proc/sys/vm/mmap_min_addr )).
    *       It may be the cas tha a mapping already exists in which the kernel
    *       decides to pick an address at its own discretion.
    * 
    * length: The amount of bytes after offset in which mmap initializes the values
    *       ,i.e. creating an appropriate virtual page. ( initialized as non-resident ).
    * 
    * prot: Describes desired mmeory protection of mapping ( of which must not conflic with open mode )
    *       of file.
    *       Following: PROT_EXEC: Pages may be executed ( forked )
    *       Prot_READ: pages may be read
    *       PROT_WRITE: Pages may be written
    *       PROT_NONE: Pages may be not accessed???
    * 
    * flags: Determins whether updates to mapping are visible to other processes mapping same region
    *        and whether updates are carried through to underlying file (interesting ).
    * 
    * offset: The location in file in which mmap is started from
    *        ( this must be a muptiple of page size which can be returned by sysconf(_SC_PAGE_SIZE)
   */
   unique_mmap( void *addr, std::size_t length, int prot, int flags, int fd, off_t offset ) : bytesMapped( length )
      {
      if ( !length )
         {
         close( fd );
         throw failure( " Error in unique_mmap constructor. length must be greater than zero! ", 
            failure::LENGTHZERO, 0 );
         } // end if
      map = mmap( addr, length, prot, flags, fd, offset );

      if ( map == MAP_FAILED )
         {
         close( fd );
         perror( "Error mmapping file" );
         throw failure( " Mmapping failed... ", errno, 0 );
         }
      }

   // Give a pointer anywhere in virtual address space...
   unique_mmap( std::size_t length, int prot, int flags, int fd, off_t offset ) : 
      unique_mmap( 0, length, prot, flags, fd, offset ) {}

   unique_mmap( void *addr, std::size_t length, int prot, int flags, off_t offset, File&& fd )
      : bytesMapped( length ), file( std::move( fd ) )
      {
      if ( !length )
         {
         throw failure( " Error in unique_mmap constructor. length must be greater than zero! ", 
            failure::LENGTHZERO, 0 );
         } // end if
      map = mmap( addr, length, prot, flags, file.getFD(), offset );

      if ( map == MAP_FAILED )
         {
         perror( "Error mmapping file" );
         throw failure( " Mmapping failed... ", errno, 0 );
         }
      }
   
   unique_mmap( unique_mmap&& other ) : map( other.map ), bytesMapped( other.bytesMapped ), file( std::move( other.file ) )
      {
      other.map = nullptr;
      }

   unique_mmap& operator=( unique_mmap&& other )
      {
      swap( other );
      return *this;
      }

   unique_mmap( const unique_mmap& ) = delete;
   unique_mmap& operator=( const unique_mmap& ) = delete;

   void swap( unique_mmap& other )
      {
      #ifdef LOCAL
            using APESEARCH::swap;
      #else
            using std::swap;
      #endif
      swap( map, other.map );
      swap( bytesMapped, other.bytesMapped );
      swap( file, other.file );
      }

   /* 
    *   Though the region is automatically unmapped when the process is terminated.  On
    *   the other hand, closing the file descriptor does not unmap the
    *   region.
    *   Note: this destructor needs to be made allowalbe to throw in case munmap fails.
   */
   ~unique_mmap( ) noexcept(false)
      {
      if ( map &&  munmap( map, bytesMapped ) == -1 )
         {
         perror( "err un-mapping the file " );
         throw failure( "error unmapping file", errno, 0 );
         } // end if
      } // end ~unique_mmap(0)

   inline void *get() const
      {
      return map;
      } // end getPointer()
   };

class HashFile
   {
   private:

      unique_mmap blob; // RAII for mmap
      File file;        // RAII for File open() => close()
      bool good = false;

      size_t FileSize( int f )
         {
         struct stat fileInfo;
         fstat( f, &fileInfo );
         return ( size_t )fileInfo.st_size;
         }

   public:

      const HashBlob *Blob( )
         {
         return reinterpret_cast< const HashBlob *> ( blob.get() );
         }

      HashFile( const char *filename ) : file( filename, O_RDONLY )
         {
         int fd = open( filename, O_RDONLY );

         blob = unique_mmap( 0, FileSize( fd ), PROT_READ, MAP_SHARED, fd, 0 );

         good = Blob()->verifyHashBlob();
         }
      
      inline bool isCorrectVersion() const { return good; }

      HashFile( const char *filename, const Hash *hashtable ) 
         : file( filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600 )
         {
         // Open the file for write, map it, write
         // the hashtable out as a HashBlob, and note
         // the blob address.

         // Your code here.
         int fd = file.getFD();
         const std::size_t bytesReq = HashBlob::BytesRequired( hashtable );
         ssize_t result = lseek( fd, off_t( bytesReq - 1 ), SEEK_SET );
         
         if ( result == -1 )
            {
            perror( "Issue with lseek while trying to stretch file" );
            return;
            } // end if
         
         result = write( fd, "", 1 );

         if ( result == -1 )
            {
            perror( "Error writing bytes to file" );
            return;
            }
         
         blob = unique_mmap( 0, bytesReq, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
         HashBlob *hb = reinterpret_cast< HashBlob *> ( blob.get() );
         HashBlob::Write( hb, bytesReq, hashtable );
         good = true;
         }

      ~HashFile( )
         {
         }
   };

#pragma GCC diagnostic pop