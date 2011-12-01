#include "FileCache.h"
#include "FileStructure.h"
#include "CompileState.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include <algorithm>

bool FileCache::QuietMode = false;

FileCache::FileCache(const std::string& aBaseDir)
: BaseDir(aBaseDir),
	ObjectsDir(""),
	DebugMode(false),
	WarnAboutNonExistingSystemHeaders(false)
{
  CompileCommand = "";
  SetObjectsShouldContainDirectories();
	IncludeDirs.push_back(".");
}

FileCache::FileCache(const FileCache& anOther)
{
}

FileCache::~FileCache()
{
}

FileCache& FileCache::operator=(const FileCache& anOther)
{
	return *this;
}

void FileCache::inDebugMode()
{
	DebugMode = true;
}

void FileCache::addPreDefine(const std::string& aName)
{
	PreDefined.push_back(aName);
}

void FileCache::generate(std::ostream& theStream, const std::string& aDirectory, const std::string& aFilename)
{
	FileStructure* Structure = update(aDirectory, aFilename, false);
	if (Structure)
	{
		CompileState State(aDirectory);
		if (DebugMode)
			State.inDebugMode();
		for (unsigned int i=0; i<PreDefined.size(); ++i)
			State.define(PreDefined[i],"");
		Structure->getDependencies(&State);
		std::string Basename(aFilename,0,aFilename.rfind("."));
		if (!ObjectsContainDirectories && Basename.rfind("/")!=Basename.npos)
		{
		    std::string newBasename(Basename,Basename.rfind("/")+1,Basename.size());
		    theStream << newBasename << ".o: " << aFilename;
		}
		else
		{
			theStream << ObjectsDir << Basename << ".o: " << aFilename;
		}
		theStream << State.getDependenciesLine() << std::endl;
//		State.dump();
		State.mergeDeps(AllDependencies);
		if (std::find(AllDependencies.begin(),AllDependencies.end(),aFilename) == AllDependencies.end())
			AllDependencies.push_back(aFilename);
		if (CompileCommand.size()>0)
		  {
		    theStream << "\t " << CompileCommand << " " << aFilename << std::endl << std::endl ;
		  }
	}
}

void FileCache::SetCompileCommand( const std::string &cmd )
{
  CompileCommand = cmd;
}

void FileCache::addIncludeDir(const std::string& aBaseDir, const std::string& aDir)
{
	if (std::find(IncludeDirs.begin(),IncludeDirs.end(),aDir) == IncludeDirs.end())
		IncludeDirs.push_back(aDir);
}

void FileCache::addExcludeDir(const std::string& aBaseDir, const std::string& aDir)
{
	if (std::find(ExcludeDirs.begin(),ExcludeDirs.end(),aDir) == ExcludeDirs.end())
		ExcludeDirs.push_back(aDir);
}

void FileCache::addIncludeDirFromFile(const std::string& aBaseDir, const std::string& aFilename)
{
    std::ifstream ifile(aFilename.c_str());
    if(!ifile){
        // XXX Error message here
        if(!QuietMode){
            std::cerr << "error opening " << aFilename.c_str() << std::endl;
        }
        // exit or return ?
        return;
    }
    while(!ifile.eof()){
        std::string aDir;
        std::getline(ifile, aDir);
		  aDir = aDir.substr(0,aDir.find('\n'));
		  aDir = aDir.substr(0,aDir.find('\r'));
		  if (aDir != "")
        	addIncludeDir(aBaseDir, aDir);
    }
}

FileStructure* FileCache::update(const std::string& aDirectory, const std::string& aFilename, bool isSystem)
{
	FileMap::iterator iti = Files.find(FileLocation(aDirectory,aFilename));
	if (iti != Files.end())
	{
/*		if(!iti->second && !QuietMode && (!isSystem || WarnAboutNonExistingSystemHeaders))
		{
			std::cerr << "error opening " << aFilename.c_str() << std::endl;
		} */
		return iti->second;
	}
	int fd = -1;
	if (DebugMode)
		std::cout << "[DEBUG] FileCache::update(" << aDirectory << "," << aFilename << ","
			<< isSystem << ");" << std::endl;
	char ResolvedBuffer[PATH_MAX+1];
	{
		unsigned int i;
		for (i=0; i<IncludeDirs.size(); ++i)
		{
			std::string theFilename = IncludeDirs[i]+'/'+aFilename;
			if (IncludeDirs[i][0] != '/')
				theFilename = BaseDir+'/'+theFilename;
			if (DebugMode)
				std::cout << "[DEBUG] realpath(" << theFilename << ",buf);" << std::endl;
			if (!realpath(theFilename.c_str(), ResolvedBuffer))
				continue;
			fd = open(ResolvedBuffer, O_RDONLY);
			if (fd >= 0)
				break;
		}
		if (i == IncludeDirs.size())
		{
			for (i=0; i<ExcludeDirs.size(); ++i)
			{
				std::string theFilename = ExcludeDirs[i]+'/'+aFilename;
				if (ExcludeDirs[i][0] != '/')
					theFilename = BaseDir+'/'+theFilename;
				if (DebugMode)
					std::cout << "[DEBUG] realpath(" << theFilename << ",buf);" << std::endl;
				if (!realpath(theFilename.c_str(), ResolvedBuffer))
					continue;
				fd = open(ResolvedBuffer, O_RDONLY);
				if (fd >= 0)
				{
					if (DebugMode)
						std::cout << "[DEBUG] excluding : " << ResolvedBuffer << std::endl;
					close(fd);
					Files[FileLocation(aDirectory, aFilename)] = 0;
					return 0;
				}
			}
			std::string theFilename = aDirectory+'/'+aFilename;
			if (aFilename[0]=='/')
			{
				theFilename = aFilename;
			}
			if (!realpath(theFilename.c_str(), ResolvedBuffer))
			{
				if(!QuietMode && (!isSystem || WarnAboutNonExistingSystemHeaders)){
					std::cerr << "error opening " << aFilename.c_str() << std::endl;
				}
				Files[FileLocation(aDirectory, aFilename)] = 0;
				return 0;
			}
			fd = open(ResolvedBuffer, O_RDONLY);
			if (fd < 0)
			{
				if(!QuietMode && (!isSystem || WarnAboutNonExistingSystemHeaders)){
					std::cerr << "error opening " << aFilename.c_str() << std::endl;
				}
				Files[FileLocation(aDirectory, aFilename)] = 0;
				return 0;
			}
		}
	}
	std::string ResolvedName(ResolvedBuffer);
	struct stat st;
	fstat(fd, &st);

	int mapsize;
	int pagesizem1 = getpagesize()-1;
	char * map;

	if (st.st_size == 0) {
		std::cerr << ResolvedName << " is empty." << std::endl;
		close(fd);
		return 0;
	}

	mapsize = st.st_size;
	mapsize = (mapsize+pagesizem1) & ~pagesizem1;
	map = (char*)mmap(NULL, mapsize, PROT_READ, MAP_PRIVATE, fd, 0);
	if ((long) map == -1) 
	{
		perror("mkdep: mmap");
		close(fd);
		return 0;
	}
	if ((unsigned long) map % sizeof(unsigned long) != 0)
	{
		std::cerr << "do_depend: map not aligned" << std::endl;
		exit(1);
	}
	FileStructure* S = new FileStructure(ResolvedName, this, map, st.st_size);
	S->setModificationTime(st.st_mtime);
	Files.insert(std::make_pair(FileLocation(aDirectory,aFilename), S));
	munmap(map, mapsize);
	close(fd);
	return S;
}

std::string FileCache::getAllDependenciesLine() const
{
	std::string Result;
	for (unsigned int i=0; i<AllDependencies.size(); ++i)
		Result += " \\\n\t"+AllDependencies[i];
	return Result;
}
