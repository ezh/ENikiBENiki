/***************************************************************************
 * Copyright (C) 2010 Alexey Aksenov, Alexx Fomichew                       *
 * Alexey Aksenov (ezh at ezh.msk.ru) software, firmware                   *
 * Alexx Fomichew (axx at fomichi.ru) hardware                             *
 *                                                                         *
 * This file is part of ENikiBENiki                                        *
 *                                                                         *
 * ENikiBENiki is free software: you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * ENikiBENiki is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with ENikiBENiki.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                         *
 ***************************************************************************/

/*
 * idea of wrapper based on source code of Matthias Braun <matze@braunis.de>
 */

#include <ptlib.h>
#include "physfs.h"

#include "Resources.h"

#define new PNEW

Resources::Resources(PString & _resourceExt) : resourceExt(_resourceExt) {
}

Resources::~Resources() {
}

bool Resources::Open(PString & argv0, PString & application) {
    if(!PHYSFS_init(argv0.GetPointer())) {
        PError << "failure while initialising physfs: " << PHYSFS_getLastError() << endl;
        return PFalse;
    } else {
        PTRACE(5, "successful initialize physfs");
    }
    const char* basedir = PHYSFS_getBaseDir();
    const char* userdir = PHYSFS_getUserDir();
    const char* dirsep = PHYSFS_getDirSeparator();
    char* writedir = new char[strlen(userdir) + application.GetLength() + 2];
    sprintf(writedir, "%s.%s", userdir, application.GetPointer());
    PTRACE(5, "physfs base directory: " << basedir);
    PTRACE(5, "physfs user directory: " << userdir);
    PTRACE(5, "physfs write directory: " << writedir);

    if(!PHYSFS_setWriteDir(writedir)) {
        // try to create the directory...
        char* mkdir = new char[application.GetLength()+2];
        sprintf(mkdir, ".%s", application.GetPointer());
        if(!PHYSFS_setWriteDir(userdir) || ! PHYSFS_mkdir(mkdir)) {
            delete[] writedir;
            delete[] mkdir;
            PError << "failed creating configuration directory: '" << writedir << "': " << PHYSFS_getLastError() << endl;
            return PFalse;
        }
        delete[] mkdir;

        if (!PHYSFS_setWriteDir(writedir)) {
            PError << "couldn't set configuration directory to '" << writedir << "': " << PHYSFS_getLastError() << endl;
            return PFalse;
        }
    }

    PHYSFS_addToSearchPath(writedir, 0);
    PHYSFS_addToSearchPath(basedir, 1);

    delete[] writedir;

    /* Root out archives, and add them to search path... */
    if (resourceExt != NULL) {
        char **rc = PHYSFS_enumerateFiles("/");
        char **i;
        size_t extlen = strlen(resourceExt);
        char *ext;

        for (i = rc; *i != NULL; i++) {
            size_t l = strlen(*i);
            if ((l > extlen) && ((*i)[l - extlen - 1] == '.')) {
                ext = (*i) + (l - extlen);
                if (strcasecmp(ext, resourceExt) == 0) {
                    PTRACE(5, "Add resource '" << *i << "' to search path");
                    const char *d = PHYSFS_getRealDir(*i);
                    char* str = new char[strlen(d) + strlen(dirsep) + l + 1];
                    sprintf(str, "%s%s%s", d, dirsep, *i);
                    addToSearchPath(str, 1);
                    delete[] str;
                };
            };
        };
        PHYSFS_freeList(rc);
    }
    return PTrue;
}

bool Resources::Close() {
    PHYSFS_deinit();
    return PTrue;
}

bool Resources::addToSearchPath(const char* directory, bool append) {
    if(!PHYSFS_addToSearchPath(directory, append)) {
        PError << "Couldn't add '" << directory << "' to searchpath: " << PHYSFS_getLastError() << endl;
        return PFalse;
    };
    return PTrue;
}

bool Resources::removeFromSearchPath(const char* directory) {
    if(!PHYSFS_removeFromSearchPath(directory)) {
        PError << "Couldn't remove '" << directory << "' from searchpath: " << PHYSFS_getLastError() << endl;
        return PFalse;
    };
    return PTrue;
}

const char* Resources::getRealDir(const char* filename) {
    return PHYSFS_getRealDir(filename);
}

std::string Resources::getRealName(const char* filename) {
    const char* dir = PHYSFS_getRealDir(filename);
    if (dir == 0) {
        PError << "no such path '" << filename << "'" << endl;
        return NULL;
    };
    std::string realname = dir;
    realname += PHYSFS_getDirSeparator();
    realname += filename;
    return realname;
}

std::string Resources::getRealWriteName(const char* filename) {
    const char* dir = PHYSFS_getWriteDir();
    if (dir == 0) {
        PError << "no writedir defined" << endl;
        return NULL;
    }
    std::string realname = dir;
    realname += PHYSFS_getDirSeparator();
    realname += filename;
    return realname;
}

char** Resources::enumerateFiles(const char* directory) {
    return PHYSFS_enumerateFiles(directory);
}

void Resources::freeList(char** list) {
    PHYSFS_freeList(list);
}

ResourceWO * Resources::RetrieveWrite(PString & filename) {
    PHYSFS_file* file = PHYSFS_openWrite(filename);
    if(!file) {
        PError << "couldn't open file '" << filename << "' for writing: " << PHYSFS_getLastError() << endl;
        return NULL;
    };
    return new ResourceWO(file);
}

ResourceRO * Resources::RetrieveRead(PString & filename) {
    PHYSFS_file* file = PHYSFS_openRead(filename);
    if(!file) {
		const int fn_length = filename.GetLength()+1;
		char fn[4096];
		memcpy(fn, filename.GetPointer(), fn_length); // includes \0;

		char * folder_sep = strrchr(fn, '/');
		char * fn_start = fn;
		char ** filelist = 0;
		if ( folder_sep ) {
			*folder_sep = 0;
			filelist = enumerateFiles(fn);
			*folder_sep = '/';
			fn_start = folder_sep+1;
		} else {
			filelist = enumerateFiles(".");
			folder_sep = fn;
		};
		if (filelist) {
			for(char** curfile = filelist; *curfile != 0; curfile++) {
				if ( strcasecmp(*curfile, fn_start) == 0 ) {
					memcpy(fn_start, *curfile, fn_length-(folder_sep-fn));
					file = PHYSFS_openRead(fn);
					break;
				};
			};
			freeList(filelist);
		};
		if (!file) {
            PError << "couldn't open file '" << filename << "' for reading: " << PHYSFS_getLastError() << endl;
            return NULL;
		};
	};
    return new ResourceRO(file);
}

ResourceWO * Resources::RetrieveAppend(PString & filename) {
    PHYSFS_file* file = PHYSFS_openAppend(filename);
    if(!file) {
        PError << "couldn't open file '" << filename << "' for writing(append): %s" << PHYSFS_getLastError() << endl;
        return NULL;
    };
    return new ResourceWO(file);
}

bool Resources::mkdir(const char* directory) {
    if(!PHYSFS_mkdir(directory)) {
        PError << "couldn't create directory '" << directory << "': " << PHYSFS_getLastError() << endl;
        return PFalse;
    };
    return PTrue;
}

bool Resources::remove(const char* filename) {
    if(!PHYSFS_delete(filename)) {
        PError << "couldn't remove file '" << filename << "': " << PHYSFS_getLastError() << endl;
        return PFalse;
    };
    return PTrue;
}

bool Resources::exists(const char* filename) {
    return PHYSFS_exists(filename);
}

bool Resources::isDirectory(const char* filename) {
    return PHYSFS_isDirectory(filename);
}

bool Resources::isSymbolicLink(const char* filename) {
    return PHYSFS_isSymbolicLink(filename);
}

int64_t Resources::getLastModTime(const char* filename) {
    int64_t modtime = PHYSFS_getLastModTime(filename);
    if (modtime < 0) {
        PError << "couldn't determine modification time of '" << filename << "': " << PHYSFS_getLastError() << endl;
        return NULL;
    };
    return modtime;
}

SDL_Surface * Resources::LoadImage(PString & imageName) {
    PTRACE(4, "load image '" << imageName << "' from resources");
    SDL_Surface * imageSurface = 0;
    ResourceRO * imageFile = RetrieveRead(imageName);
    if (!imageFile)
        return NULL;
    SDL_RWops * rw = imageFile->GetSDLRWOps();
    if (!rw)
        return NULL;
    imageSurface = SDL_LoadBMP_RW(rw, 0);
    PTRACE(4, "image width: " << imageSurface->w << ", image height: " << imageSurface->h);
    return imageSurface;
}

SDL_Surface * Resources::LoadImageOptimized(PString & imageName) {
    SDL_Surface * imageSurface = 0;
    SDL_Surface * optimizedSurface = 0;
    
    imageSurface = LoadImage(imageName);
    if (!imageSurface)
        return NULL;
    //Create an optimized image
    optimizedSurface = SDL_DisplayFormat(imageSurface);
    //Free the old surface
    SDL_FreeSurface(imageSurface);
    return optimizedSurface;
}

bool Resources::LoadTextFile(PString & fileName, PStringArray & textBuffer) {
    PTRACE(4, "load textfile '" << fileName << "' from resources");
    bool result;
    ResourceRO * textFile = RetrieveRead(fileName);
    result = textFile->readText(textBuffer);
    delete textFile;
    return result;
}

//int * ResFile::loadFile(char *fileName, int *fileSize) {
/*    PHYSFS_file *compFile;
    PHYSFS_sint64 fileLength;
    int *levelInMem = NULL;
    char tempFilename[MAX_STRING_SIZE];
    
    if (NO == fileSystemReady) {
        SDL_SetError("PHYSFS system has not been initialised.");
        return NULL;
    }
    
    // Get a handle to the file
    compFile = PHYSFS_openRead(fileName);
    if (NULL == compFile) {
        strcpy(tempFilename, "");
        strcpy(tempFilename, dataDirectory);
        strcat(tempFilename, fileName);
        compFile = PHYSFS_openRead(fileName);
        if (NULL == compFile)
            return NULL;
    }
    // Get it's size
    fileLength = PHYSFS_fileLength(compFile);
    if (-1 == fileLength) {
        //CON_Out (mainConsole, "Unable to determine file length for [ %s ]", fileName);
        return NULL;
    }
    // Pass the filesize back
    *fileSize = (int)fileLength;
    //      conPrint (true, "Size of [ %s ] is [ %i ]", fileName, fileLength);
    if (NULL != levelInMem)
        free(levelInMem);
    levelInMem = (int *)malloc((int)fileLength);
    int returnCode = (int)PHYSFS_read(compFile, (void *)levelInMem, (PHYSFS_uint32)fileLength, 1);
    if (-1 == returnCode) {
        return NULL;
    };
    PHYSFS_close(compFile);
    return (int *)levelInMem;*/
//    return 0;
//}

