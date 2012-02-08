#ifndef _included_filesystem_h
#define _included_filesystem_h


/*
 * ===========
 * FILE SYSTEM
 * ===========
 *
 *	A file access system designed to allow an app 
 *  to open files for reading/writing.
 *  Will transparently load from compressed archives 
 *  if they are present.
 *
 */

class MemMappedFile;
class TextReader;
class BinaryReader;

#include "lib/tosser/btree.h"
#include "lib/tosser/llist.h"



class FileSystem 
{
protected:
	BTree <MemMappedFile *>	m_archiveFiles;

    char const * m_writePath;
public:
    LList <char *> m_searchPath;                                                        // Use to set up mods

public:
    FileSystem();
    ~FileSystem();


    void            ParseArchive		( const char *_filename );
    void            ParseArchives		( const char *_dir, const char *_filter );

	TextReader		*GetTextReader	    ( const char *_filename );	                    // Caller must delete the TextReader when done
	TextReader		*GetTextReaderDefault( const char *_filename );	                    // Ignores MODs
	BinaryReader	*GetBinaryReader    ( const char *_filename );	                    // Caller must delete the BinaryReader when done
	BinaryReader	*GetBinaryReaderDefault( const char *_filename );	                // Ignores MODs

    LList<char *>   *ListArchive        (char *_dir, char *_filter, bool fullFilename = true);


    void            ClearSearchPath     ();
    void            AddSearchPath       ( char const *_path );                          // Must be added in order

    void            SetWritePath        ( char const * writePath );
    char const *    GetWritePath        () const;
};




extern FileSystem *g_fileSystem;

#endif
