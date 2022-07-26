/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemFile.h"

#include "InitFinisManager.h"
#include "MmapStream.h"
#include "SystemBase.h"
#include "SystemLog.h"
#include "Tracer.h"
#include "singleton.hpp"

#include <climits>
#include <cstring>
#include <fstream>

#include <errno.h>
#include <sys/stat.h>
#if defined(WIN32)
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>	 // directory access
#include <sys/statvfs.h>
#include <unistd.h>
#endif

namespace {
	//! contains the systems path separator
	const char cSep = '/';

//! defines maximal size of path
#if defined(_POSIX_PATH_MAX)
	const long cPATH_MAX = _POSIX_PATH_MAX;
#else
	const long cPATH_MAX = PATH_MAX;
#endif

	class PathInitializer {
		//! contains the root directory path that is used to locate files, it can be relative or absolute
		String fgRootDir;
		//! contains a search path list that is ':' delimited, it is used to search for files
		String fgPathList;

	public:
		PathInitializer() : fgRootDir(0L, coast::storage::Global()), fgPathList(0L, coast::storage::Global()) {
			InitFinisManager::IFMTrace("PathInitializer::Initialized\n");
			initialize(0, 0);
		}
		~PathInitializer() { InitFinisManager::IFMTrace("PathInitializer::Finalized\n"); }
		void initialize(const char *root, const char *path) {
			String tmpRoot((root) != 0 ? String(root) : coast::system::EnvGet("COAST_ROOT"));
			String tmpPath((path) != 0 ? String(path) : coast::system::EnvGet("COAST_PATH"));
			static bool once = false;
			if (!once || tmpRoot.Length() > 0 || tmpPath.Length() > 0) {
				once = true;
				if (not tmpRoot.empty()) {
					SetRootDir(tmpRoot, true);
				} else {
					SetRootDir(".", true);
				}
				if (not tmpPath.empty()) {
					SetPathList(tmpPath, true);
				} else {
					SetPathList(".:config:src:", true);
				}
			}
		}
		void SetRootDir(const char *root, bool print) {
			if (root != 0) {
				String rPath(root);
				// when a dot is supplied as root, ResolvePath would remove it and leave the path blank
				if (rPath != ".") {
					coast::system::ResolvePath(rPath);
				}
				if (print) {
					SystemLog::WriteToStderr(String("Root Dir: <") << rPath << ">"
																   << "\n");
				}
				String logMsg("Root Dir: <");
				logMsg << rPath << ">";
				SystemLog::Info(logMsg);
				fgRootDir = rPath;
			}
		}
		void SetPathList(const char *pathlist, bool print) {
			if (pathlist != 0) {
				if (print) {
					SystemLog::WriteToStderr(String("Pathlist: <") << pathlist << ">"
																   << "\n");
				}
				String logMsg("Pathlist: <");
				logMsg << pathlist << ">";
				SystemLog::Info(logMsg);
				fgPathList = pathlist;
			}
		}
		const char *GetRootDir() { return fgRootDir.cstr(); }

		const char *GetPathList() { return fgPathList.cstr(); }
	};
	typedef coast::utility::singleton_default<PathInitializer> PathInitializerSingleton;

	//! checks existence of a path using stat
	/*! \param path file or directory path
	  \param stbuf stat buffer to fill
	  \param bIsSymbolicLink set to true if the given path is a link pointing either to a file or directory. The st_mode member of the stat param will be set to the real type the link points to.
	  \return true in case the call to stat was successful */
	bool CheckPath(const char *path, struct stat64 *stbuf, bool &bIsSymbolicLink) {
		StartTrace(System.CheckPath);
		bool result = false;
#if defined(WIN32)	//! TODO: symlinks seem to be supported since vista
		bIsSymbolicLink = false;
#else
		while (!(result = (lstat64(path, stbuf) == 0)) && coast::system::SyscallWasInterrupted()) {
			String msg("OOPS, lstat failed with ");
			msg << SystemLog::LastSysError() << " on " << path;
			Trace(msg);
			SYSWARNING(msg);
		}
		bIsSymbolicLink = (result ? S_ISLNK(stbuf->st_mode) : false);
		if (result) {
			Trace("mode field value of lstat: " << (long)stbuf->st_mode);
		}
#endif
		while (!(result = (stat64(path, stbuf) == 0)) && coast::system::SyscallWasInterrupted()) {
			String msg("OOPS, stat failed with ");
			msg << SystemLog::LastSysError() << " on " << path;
			Trace(msg);
			SYSWARNING(msg);
		}
		if (result) {
			Trace("mode field value of stat: " << (long)stbuf->st_mode);
		}
		return result;
	}

	//! bottleneck routine which opens the stream if possible it returns null if not successful
	/*! \param path the filepath, it can be relative or absolute, it contains the extension
	  \param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
	  \param trace if true writes messages to SystemLog
	  \return an open iostream or NULL if the open fails */
	std::iostream *DoOpenStream(const char *path, coast::system::openmode mode, bool log = false) {
		StartTrace1(System.DoOpenStream, "file [" << NotNull(path) << "]");
		// adjust mode to output, append implies it
		if ((mode & std::ios::app) != 0) {
			mode |= std::ios::out;
		}

#if defined(WIN32)
		// std::in is needed in win32 to force std::ate to do what it was meant to do
		if (mode & std::ios::ate) {
			mode |= std::ios::in;
		}
#endif

		if (coast::system::IsRegularFile(path) || ((mode & std::ios::out) != 0)) {
			// ios::ate is special, it should only work on existing files according to standard
			// so must set the out flag here and not earlier
			if ((mode & std::ios::ate) != 0) {
				mode |= std::ios::out;
			}
#if !defined(WIN32)
			static bool bUseMmapStreams = (coast::system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L);
			if (bUseMmapStreams) {
				MmapStream *fp = new MmapStream(path, mode);
				if (fp->good() && fp->rdbuf()->is_open()) {
					return fp;
				}
				Trace("failed to open MmapStream, file [" << path << "]");
				delete fp;
			} else {
#endif
				std::fstream *fp = new std::fstream(path, (coast::system::openmode)mode);
				if (fp->good() && fp->rdbuf()->is_open()) {
					return fp;
				}
				Trace("failed to open fstream. file [" << path << "]");
				delete fp;
#if !defined(WIN32)
			}
#endif
		} else {
			if (log) {
				SYSWARNING(String("file [") << path << "] does not exist or is not accessible");
			}
		}
		return 0;
	}

	//! internal method which opens a stream if possible
	/*! \param path the filepath, it can be relative or absolute, it contains the extension
	  \param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
	  \return an open iostream or NULL if the open fails */
	std::iostream *IntOpenStream(const String &path, coast::system::openmode mode) {
		StartTrace(System.IntOpenStream);

		String filepath(path);

		// fast exit
		if (filepath.Length() == 0L) {
			return 0;
		}

		coast::system::ResolvePath(filepath);

		if (coast::system::IsAbsolutePath(filepath)) {
			Trace("file [" << filepath << "] is going to be opened absolute");
		} else {
			Trace("file [" << filepath << "] is going to be opened relative");
		}

		std::iostream *Ios = DoOpenStream(filepath, mode);

		if (Ios != 0) {
			Trace(filepath << " found");
		} else {
			Trace("can't open " << filepath);
		}
		if (Ios == NULL) {
			Trace("couldn't open file [" << filepath << "] searched in [" << coast::system::GetRootDir() << "] with pathlist: ["
										 << coast::system::GetPathList() << "]");
		}

		return Ios;
	}

	//! internal method which searches a pathlist for the location of a file
	/*! \param name the filename (including extension), must be relative
	  \param path the pathlist to be searched, relative to the root directory
	  \return the location of the file found (empty if file was not found) */
	String searchFilePath(const char *name, const char *path) {
		StartTrace(System.searchFilePath);

		StringTokenizer st(path, ':');
		String dirpath, filepath;
		// search over pathlist
		while (st.NextToken(dirpath)) {
			filepath.Append(coast::system::GetRootDir())
				.Append(coast::system::Sep())
				.Append(dirpath)
				.Append(coast::system::Sep())
				.Append(name);
			coast::system::ResolvePath(filepath);
			Trace("trying [" << filepath << "]");

			if (coast::system::IsRegularFile(filepath) && coast::system::io::access(filepath, R_OK) == 0) {
				return filepath;
			}
			// reset filepath and try next path
			filepath.clear();
		}
		return filepath;
	}

	String buildFilename(const char *name, const char *extension) {
		String filename = name;
		if ((extension != 0) && strlen(extension) > 0) {
			filename.Append('.').Append(extension);
		}
		return filename;
	}

#if !defined(WIN32)
	//! internal method to extend a directory with given permissions by creating 'extension' directories and softlinks
	/*!	\param path relative or absolute path to create new directory
	  \param pmode permission of new directory, octal number
	  \return system::eSuccess if new directory was created */
	coast::system::DirStatusCode IntExtendDir(String &strOriginalDir, int pmode) {
		StartTrace1(System.IntExtendDir, "dir to create [" << strOriginalDir << "]");
		coast::system::DirStatusCode aDirStatus(coast::system::ePathIncomplete);
		String strBasePath(strOriginalDir), strDirToExtend, strFinalDir, strExtensionDir;
		long slPos = strBasePath.StrRChr(coast::system::Sep());
		if (slPos > 0L) {
			// find final segment
			strFinalDir = strBasePath.SubString(slPos + 1);
			strBasePath.Trim(slPos);
			// find path to extend by other dir and link
			slPos = strBasePath.StrRChr(coast::system::Sep());
			if (slPos > 0L) {
				// find final segment
				strDirToExtend = strBasePath.SubString(slPos + 1);
				strBasePath.Trim(slPos);
			}
		}
		bool bContinue = (strFinalDir.Length() > 0L && strDirToExtend.Length() > 0L);
		long lExt = 0L;
		while (bContinue) {
			strExtensionDir = strBasePath;
			if (strExtensionDir.Length() > 0L) {
				strExtensionDir.Append(coast::system::Sep());
			}
			strExtensionDir.Append(strDirToExtend).Append("_ex").Append(lExt).Append(coast::system::Sep()).Append(strFinalDir);
			Trace("trying extension directory [" << strExtensionDir << "]");
			switch (aDirStatus = coast::system::MakeDirectory(strExtensionDir, pmode, true, false)) {
				case coast::system::eNoMoreHardlinks: {
					// no more room for creating a 'real' directory
					// lets try the next extension number
					break;
				}
				case coast::system::eExists:
					// need to catch this, directory might exist already or someone else created it recently
				case coast::system::eSuccess: {
					// create link into original directory (strOriginalDir)
					aDirStatus = coast::system::CreateSymbolicLink(strExtensionDir, strOriginalDir);
				} /* fall through */
				default: {
					bContinue = false;
				}
			}
			++lExt;
		}
		return aDirStatus;
	}
#endif

	//! internal method to create new directory with given permissions, works for relative or absolute path names and also
	//! recursive if specified.
	/*! \note Unless bRecurse is true, the parent directory must already exist and only one directory level will be created
	  \param path relative or absolute path to create new directory
	  \param pmode permission of new directory, octal number
	  \param bRecurse set to true if nonexisting parent directories should be created
	  \param bExtendByLinks if a directory can not be created because its parent dir is exhausted of hard links (subdirectories), a true means to create an 'extension' parent directory of name <dir>_ex[0-9]+ and link the newly created directory into the original location
	  \return system::eSuccess if new directory was created */
	coast::system::DirStatusCode IntMakeDirectory(String &path, int pmode, bool bRecurse, bool bExtendByLinks) {
		StartTrace1(System.IntMakeDirectory, "dir to create [" << path << "], recurse " << (bRecurse ? "true" : "false"));
		String strParentDir;
		long slPos = path.StrRChr(coast::system::Sep());
		if (slPos > 0L) {
			// remove last segment and slash
			strParentDir = path.SubString(0L, slPos);
		}
		Trace("parent path [" << strParentDir << "]");

		coast::system::DirStatusCode aDirStatus = coast::system::ePathIncomplete;
		if (strParentDir.Length() != 0) {
			if (coast::system::IsDirectory(strParentDir)) {
				aDirStatus = coast::system::eSuccess;
			} else {
				if (bRecurse) {
					// recurse to make parent directories
					aDirStatus = IntMakeDirectory(strParentDir, pmode, bRecurse, bExtendByLinks);
				} else {
					SYSERROR("parent directory does not exist [" << strParentDir << "]");
					aDirStatus = coast::system::eNotExists;
				}
			}
		} else {
			// assume root level or relative
			aDirStatus = coast::system::eSuccess;
		}
		// eExists might occur if someone created the parent directory in parallel
		if (aDirStatus == coast::system::eSuccess || aDirStatus == coast::system::eExists) {
			// make new directory
			if (coast::system::io::mkdir(path, pmode) != 0L) {
				switch (
					errno) {  // errno needs to be used in comparisons
							  // if errno is set to EEXIST, someone else might already have created the dir, so do not complain
					case EEXIST: {
						SYSINFO("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError()
											 << "] because the directory was created by someone else in the meantime?!");
						aDirStatus = coast::system::eExists;
						break;
					}
#if !defined(WIN32)
					case EDQUOT: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::eQuotaExceeded;
						break;
					}
#endif
					case ENOSPC: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError()
											  << "] -> check for free inodes using $>df -F ufs -o i <FS>");
						aDirStatus = coast::system::eNoSpaceLeft;
						break;
					}
					case EACCES: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::eNoPermission;
						break;
					}
					case ENOENT: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::eNotExists;
						break;
					}
					case ENOTDIR: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::eNotDirectory;
						break;
					}
					case EIO: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::eIOOperationFailed;
						break;
					}
					case EFAULT: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::ePathIllegal;
						break;
					}
					case EMLINK: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError()
												<< "] because the parent directory is exhausted of hardlinks!");
						aDirStatus = coast::system::eNoMoreHardlinks;
#if !defined(WIN32)
						if (bExtendByLinks) {
							// if no more directories can be created, we use 'extension' links instead
							aDirStatus = IntExtendDir(path, pmode);
						}
#endif
						break;
					}
					default: {
						SYSERROR("mkdir of [" << path << "] failed with [" << SystemLog::LastSysError() << "]");
						aDirStatus = coast::system::eCreateDirFailed;
					}
				}
			}
		} else {
			// assign parent directory to parameter to be able to localize the problem
			path = strParentDir;
		}
		return aDirStatus;
	}

	//! internal method to remove given directory - relative directories can be deleted recursively
	/*! \param path relative or absolute path of directory to be deleted
	  \param bRecurse true if relative directory should be deleted recusrively
	  \param bAbsDir if true we can not recurse to delete directories
	  \return system::eSuccess if directory was removed */
	coast::system::DirStatusCode IntRemoveDirectory(String &path, bool bRecurse, bool bAbsDir) {
		StartTrace1(System.IntRemoveDirectory, "dir to remove [" << path << "], recurse " << (bRecurse ? "true" : "false"));

		coast::system::DirStatusCode aDirStatus = coast::system::eNoSuchFileOrDir;
		if ((coast::system::IsDirectory(path) && coast::system::io::rmdir(path) == 0L) ||
			(coast::system::IsSymbolicLink(path) && coast::system::io::unlink(path) == 0L)) {
			aDirStatus = coast::system::eSuccess;
			if (bRecurse && !bAbsDir) {
				long slPos = path.StrRChr(coast::system::Sep());
				if (slPos > 0L) {
					// remove parent dir
					String strParent(path.SubString(0L, slPos));
					aDirStatus = IntRemoveDirectory(strParent, bRecurse, bAbsDir);
					if (aDirStatus != coast::system::eSuccess) {
						path = strParent;
					}
				}
			}
		} else {
			SYSERROR("rmdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
			switch (errno) {  // errno needs to be used in comparisons
				case EEXIST:
				case ENOTEMPTY:
					aDirStatus = coast::system::eExists;
					break;
				case ENOENT:
					aDirStatus = coast::system::eNotExists;
					break;
				default:;
			}
		}
		return aDirStatus;
	}

}  // namespace

namespace coast {
	namespace system {

		char Sep() { return cSep; }

		void InitPath(const char *root, const char *path) { PathInitializerSingleton::instance().initialize(root, path); }

		void SetRootDir(const char *root, bool print) { PathInitializerSingleton::instance().SetRootDir(root, print); }

		void SetPathList(const char *pathlist, bool print) {
			PathInitializerSingleton::instance().SetPathList(pathlist, print);
		}

		// used to smoothify the given path;
		// remove relative movements
		// on WIN32, convert to correct drive notation and use only one kind of slashes
		void ResolvePath(String &path) {
			if (path.empty()) {
				return;
			}
			// remember where the real path segmnent starts, needed especially for WIN32
			String drive;
#if defined(WIN32)
			// get drive letter if any specified
			char drv;
			if (GetDriveLetter(path, drv)) {
				drive.Append(drv);
				drive.Append(':');
				StripDriveLetter(path);
			}
#endif
			String newPath;
			if (IsAbsolutePath(path)) {
				newPath.Append('/');
			}

#if defined(WIN32)
			// only use one kind of slashes, this simplifies the following code
			long sidx;
			while ((sidx = path.StrChr('\\')) != -1) {
				path.PutAt(sidx, '/');
			}
#endif

			// process the given path, resolve all relative path movements
			// and use it to create the new path
			String token, nxtoken;
			StringTokenizer myTok(path, '/');
			bool boTok = myTok(token);
			while (boTok) {
				boTok = myTok(nxtoken);
				if (token.IsEqual(".")) {
					// skip current path tokens
				} else if (token.IsEqual("..")) {
					long idx = newPath.StrRChr('/');
					if (idx == 0) {
						// do not go beyond root, trim to first slash
						// root=/, move=../c should yield /c
						newPath.Trim(1);
					} else if (idx > 0) {
						// must be a valid path segment which we can remove
						// just remove the last path segment because the back movement made it obsolete
						newPath.Trim(idx);
					} else {
						// probably no slash found, seems to be a relative path
						// reset the path because the back movement made it obsolete
						newPath.clear();
					}
				} else if (token.empty()) {
					// skip empty token
					if (!boTok && static_cast<long>(not newPath.empty()) > 0L && newPath[newPath.Length() - 1L] != '/') {
						newPath.Append('/');
					}
				} else {
					// regular token
					if (not newPath.empty() && newPath[newPath.Length() - 1L] != '/') {
						newPath.Append('/');
					}
					newPath.Append(token);
				}
				token = nxtoken;
				nxtoken.clear();
			}
			if (not drive.empty()) {
				// prepend drive letter if any
				drive.Append(newPath);
				newPath = drive;
			}
			path = newPath;
		}

		void StripDriveLetter(String &path) {
#if defined(WIN32)
			// <drive-letter:>, standard NT syscalls return this notation
			// <//drive-letter>, some U*x-shells on WIN32 use this notation
			if ((path.Length() > 1) && isalpha(path[0L]) && path[1L] == ':') {
				path.TrimFront(2);
			} else if ((path.Length() > 2) && path[0L] == '/' && path[1L] == '/' && isalpha(path[2L])) {
				path.TrimFront(3);
			}
#endif
		}

		bool GetDriveLetter(const char *name, char &drive) {
#if defined(WIN32)
			// <drive-letter:>, standard NT syscalls return this notation
			// <//drive-letter>, some U*x-shells on WIN32 use this notation
			String str(name);
			if ((str.Length() > 1) && isalpha(str[0L]) && str[1L] == ':') {
				drive = str[0L];
				return true;
			} else if ((str.Length() > 2) && str[0L] == '/' && str[1L] == '/' && isalpha(str[2L])) {
				drive = str[2L];
				return true;
			}
#endif
			return false;
		}

		bool IsAbsolutePath(const char *name) {
			if (name != 0) {
				// check for absolute path names
#if defined(WIN32)
				// following absolute paths could be possible on WIN32
				// <drive-letter:\>, standard NT syscalls return this notation
				// <//drive-letter/>, some U*x-shells on WIN32 use this notation
				// <\> and </> is absolute too
				if (((strlen(name) > 2) && isalpha(name[0]) && name[1] == ':' && (name[2] == '/' || name[2] == '\\')) ||
					((strlen(name) > 3) && name[0] == '/' && name[1] == '/' && isalpha(name[2]) && name[3] == '/') ||
					(*name) == '\\' || (*name) == '/') {
					// under WIN32 it might start with or
					// without <drive-letter:>
					return true;
				}
#else
				if ((*name) == Sep()) {
					// it is an absolute path name
					return true;
				}
#endif
			}
			return false;
		}

		void GetCWD(String &cwd) {
			String buf(4096L);
			cwd = ".";
			if (getcwd((char *)buf.cstr(), buf.Capacity()) == 0) {
				SystemLog::Alert("puh, cannot obtain current working directory");
				return;
			}
			cwd = String((void *)buf.cstr(), strlen(buf.cstr()));
			ResolvePath(cwd);
		}

		bool ChangeDir(const String &dir) {
			if (dir.Length() > 0 && dir != ".") {
				if (!IsDirectory(dir) || chdir(dir) != 0) {
					return false;
				}
			}
			return true;
		}

		int Chmod(const char *filename, int pmode) {
#if defined(WIN32)
			return _chmod(filename, pmode);
#else
			return ::chmod(filename, pmode);
#endif
		}

		std::iostream *OpenIStream(const char *name, const char *extension, openmode mode, bool log) {
			return OpenIStream(buildFilename(name, extension), mode);
		}

		std::iostream *OpenOStream(const char *name, const char *extension, openmode mode, bool log) {
			return OpenOStream(buildFilename(name, extension), mode);
		}

		std::iostream *OpenStream(const char *name, const char *extension, openmode mode, bool log) {
			return OpenStreamWithSearch(buildFilename(name, extension), mode);
		}

		std::iostream *OpenIStream(const String &path, openmode mode) { return IntOpenStream(path, mode | std::ios::in); }

		std::iostream *OpenOStream(const String &path, openmode mode) { return IntOpenStream(path, mode | std::ios::out); }

		std::iostream *OpenStream(const String &path, openmode mode) { return IntOpenStream(path, mode); }

		std::iostream *OpenStreamWithSearch(const String &path, openmode mode) {
			return IntOpenStream(GetFilePathOrInput(path), mode);
		}

		String GetFilePath(const char *name, const char *extension) {
			StartTrace(System.GetFilePath);

			String resultPath;

			// fast exit
			if (name == 0) {
				return resultPath;
			}

			String relpath(name);
			if ((extension != 0) && strlen(extension) > 0) {
				relpath.Append('.').Append(extension);
			}

			return GetFilePath(relpath);
		}

		String GetFilePath(const String &relpath) {
			StartTrace(System.GetFilePath);

			String resultPath;

			// fast exit
			if (relpath.Length() == 0L) {
				return resultPath;
			}

			if (IsAbsolutePath(relpath)) {
				Trace("absolute path given [" << relpath << "]");
				resultPath = relpath;
			} else {
				Trace("file [" << relpath << "] is going to be searched in [" << GetRootDir() << "] with pathlist: ["
							   << GetPathList() << "]");
				resultPath = searchFilePath(relpath, GetPathList());
			}

			if (resultPath.Length() > 0L) {
				Trace("path found: [" << resultPath << "]");
			} else {
				Trace("couldn't find file [" << relpath << "] searched in [" << GetRootDir() << "] with pathlist: ["
											 << GetPathList() << "]");
			}

			ResolvePath(resultPath);
			return resultPath;
		}

		String GetFilePathOrInput(const String &relpath) {
			String foundpath = GetFilePath(relpath);
			if (foundpath.Length() > 0L) {
				return foundpath;
			}
			return relpath;
		}

		String GetTempPath() {
#if defined(WIN32)
			TCHAR lpTempPathBuffer[MAX_PATH];
			DWORD dwRetVal = 0;
			dwRetVal = ::GetTempPath(MAX_PATH, lpTempPathBuffer);
			if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
				// error
			}
			return String(lpTempPathBuffer);
#else
			return "/tmp";
#endif
		}

		bool FindFile(String &fullPathName, const char *file, const char *path) {
			StartTrace(System.FindFile);
			bool bRet = false;
#if defined(WIN32)
			char *pNamePos = NULL;
			String envPATH;
			if (path == NULL) {
				// no search path given, default is to use PATH variable
				envPATH = EnvGet("PATH");
				Trace("using sysPath [" << envPATH << "]");
				path = envPATH.cstr();
			}
			// returns needed buffersize exclusive terminating null
			long lBufSize = SearchPath(path,	  // search path, if NULL the system searches the file
									   file,	  // file name including extension
									   NULL,	  // extension, we do not use this notation
									   0,		  // buffersize for the resulting path, let the function tell us how long it is
									   NULL,	  // char* to buffer
									   &pNamePos  // pointer to position of filename within buffer
			);
			if (lBufSize > 0) {
				Trace("buffersize needed:" << lBufSize);
				char *buffer = new char[lBufSize++];
				// and again with the allocated buffer
				lBufSize = SearchPath(path,		 // search path, if NULL the system searches the file
									  file,		 // file name including extension
									  NULL,		 // extension, we do not use this notation
									  lBufSize,	 // buffersize for the resulting path, let the function tell us how long it is
									  buffer,	 // char* to buffer
									  &pNamePos	 // pointer to position of filename within buffer
				);
				Trace("name from SearchPath [" << buffer << "]");
				fullPathName = buffer;
				delete[] buffer;
				ResolvePath(fullPathName);
				Trace("Normalized pathname [" << fullPathName << "]");
				bRet = true;
			} else {
				// some error occured, set a warning
				SYSWARNING(String("tried to find [") << file << "], error is:" << SystemLog::LastSysError());
			}
#else
			// HUM: found this somewhere but don't know how to make functional on Solaris/Linux
//	char* pFullPath = NULL;
//	String envPATH;
//	if (path == NULL)
//	{ // no search path given, default is to use PATH variable
//		envPATH = EnvGet("PATH");
//		Trace("using sysPath [" << envPATH << "]");
//		path = (const char*)envPATH;
//	}
//	String strPathBuffer(256L);
//	pFullPath = pathfind_r(
//		path,		// search path
//        file,		// filename to lookup
//        "rx",		// mode flags, we need read and execute
//		(char *)(const char*)strPathBuffer,	// pointer to character buffer
//		256L		// buffersize
//	);
//	if (pFullPath != NULL)
//	{ // found the file
//		Trace(String("Normalized pathname [") << strPathBuffer << "]");
//		fullPathName = strPathBuffer;
//		bRet = true;
//	}
//	else
//	{ // some error occured, set a warning
//		SYSWARNING(String("tried to find [") << file << "], error is:" << SystemLog::LastSysError());
//	}
#endif
			return bRet;
		}

		bool IsRegularFile(const char *path) {
			struct stat64 stbuf;
			bool bIsSymbolicLink = false;
			if (CheckPath(path, &stbuf, bIsSymbolicLink)) {
#if defined(WIN32)
				// our version of MSDEV compiler (V 5) at least not fully
				// posix compatible.
				return ((stbuf.st_mode & _S_IFMT) == _S_IFREG);
#else
				return S_ISREG(stbuf.st_mode);
#endif
			}
			return false;
		}

		bool IsDirectory(const char *path) {
			struct stat64 stbuf;
			bool bIsSymbolicLink = false;
			if (CheckPath(path, &stbuf, bIsSymbolicLink)) {
#if defined(WIN32)
				// our version of MSDEV compiler (V 5) at least not fully
				// posix compatible.
				return ((stbuf.st_mode & _S_IFMT) == _S_IFDIR);
#else
				return S_ISDIR(stbuf.st_mode);
#endif
			}
			return false;
		}

		bool IsSymbolicLink(const char *path) {
			struct stat64 stbuf;
			bool bIsSymbolicLink = false;
			if (CheckPath(path, &stbuf, bIsSymbolicLink)) {
				return bIsSymbolicLink;
			}
			return false;
		}

		bool GetFileSize(const char *path, ul_long &ulFileSize) {
			StartTrace1(System.GetFileSize, "path to file [" << NotNull(path) << "]");
			struct stat64 stbuf;
			bool bIsSymbolicLink = false;
			if (CheckPath(path, &stbuf, bIsSymbolicLink)) {
				ulFileSize = stbuf.st_size;
				Trace("file size: " << static_cast<long>(ulFileSize) << " bytes");
				return true;
			}
			return false;
		}

		const char *GetRootDir() { return PathInitializerSingleton::instance().GetRootDir(); }

		const char *GetPathList() { return PathInitializerSingleton::instance().GetPathList(); }

		Anything DirFileList(const char *dir, const char *extension) {
			StartTrace(System.DirFileList);
			Anything list;
			//  PS: not needed because we have a default parameter....
			//	if (!extension)
			//		extension= "html";
			// PS: I require more than that....
			String strExtension;
			if ((extension != 0) && strlen(extension) >= 1) {  // allow one to specify "" for getting all files
				strExtension.Append('.').Append(extension);
			}
			long truncext = strExtension.Length();
#if defined(WIN32)
			// messy directory interface
			WIN32_FIND_DATA FileData;
			HANDLE hSearch;
			String searchPath(dir);
			BOOL fFinished = false;

			searchPath << Sep() << "*" << strExtension;
			// Start searching for (.html) files in the directory.
			hSearch = FindFirstFile(searchPath.cstr(), &FileData);
			if (hSearch == INVALID_HANDLE_VALUE) {
				return list;
			}
			// Copy each .TXT file to the new directory
			// and change it to read only, if not already.
			while (!fFinished) {
				const char *fName = FileData.cFileName;
				list.Append(Anything(fName, strlen(fName) - truncext));	 // PS: warum die extension abschneiden?

				if (!FindNextFile(hSearch, &FileData)) {
					if (GetLastError() == ERROR_NO_MORE_FILES) {
						fFinished = true;
					} else {
						SYSWARNING("couldn't find file");
					}
				}
			}
			// Close the search handle.
			FindClose(hSearch);
#else
			// searches directory dir and
			DIR *fp = NULL;

			if ((fp = opendir(dir)) != 0) {
				// do not use Storage module to allocate memory here, since readdir_r wreaks havoc with our storage management
				struct dirent *direntp = (dirent *)calloc(1, sizeof(dirent) + cPATH_MAX);
				struct dirent *direntpSave = direntp;
				while ((readdir_r(fp, direntp, &direntp) == 0) && ((direntp) != 0)) {
					String name = direntp->d_name;
					Trace("current entry [" << name << "]");
					long start = name.Length() - truncext;
					if ((start > 0) && strcmp(name, ".") != 0 && strcmp(name, "..") != 0 &&
						(name.CompareN(strExtension, truncext, start) == 0)) {
						// append filename without extension
						Trace("appending entry [" << name.SubString(0, start) << "]");
						list.Append(Anything(name, start));
					}
				}
				if (direntp != 0) {
					Trace("last entry [" << NotNull(direntp->d_name) << "]");
				}
				::free(direntpSave);
				closedir(fp);
			} else {
				String logMsg("can't open ");
				logMsg.Append(dir);
				SYSERROR(logMsg);
			}
#endif
			return list;
		}

		bool BlocksLeftOnFS(const char *pFsPath, ul_long &ulBlocks, unsigned long &lBlkSize) {
			StartTrace1(System.BlocksLeftOnFS, "fs path [" << NotNull(pFsPath) << "]");

#if defined(WIN32)
			_ULARGE_INTEGER ulBytesAvailable;
			if (GetDiskFreeSpaceEx(pFsPath, &ulBytesAvailable, NULL, NULL) != 0) {
				lBlkSize = 1;
				ulBlocks = ulBytesAvailable.QuadPart;
				Trace("blocksize: " << static_cast<long>(lBlkSize) << " bytes free blocks: " << static_cast<long>(ulBlocks));
				return true;
			} else
#else
			struct statvfs buf;
			if (0 == statvfs(pFsPath, &buf)) {
				lBlkSize = (unsigned long)buf.f_frsize;
				ulBlocks = (ul_long)buf.f_bavail;
				Trace("blocksize: " << static_cast<long>(lBlkSize) << " bytes free blocks: " << static_cast<long>(ulBlocks));
				return true;
			}
#endif
				SYSWARNING("Failed to get blocks left on FS [" << SystemLog::LastSysError() << "]");

			return false;
		}

		DirStatusCode MakeDirectory(String &path, int pmode, bool bRecurse, bool bExtendByLinks) {
			StartTrace1(System.MakeDirectory, "dir to create [" << path << "], recurse " << (bRecurse ? "true" : "false")
																<< ", extendbylinks " << (bExtendByLinks ? "true" : "false"));
			// cleanup relative movements and slashes
			ResolvePath(path);

			// check for empty input parameter
			if (path.Length() == 0) {
				SYSWARNING("empty path given, bailing out!");
				return ePathEmpty;
			}
			// check if directory already exists
			if (IsDirectory(path)) {
				SYSWARNING("directory already exists [" << path << "]");
				return eExists;
			}
			// check whether path-length is ok
			if (path.Length() > cPATH_MAX) {
				SYSERROR("Path size exceeded [" << path << "]");
				return ePathTooLong;
			}
			return IntMakeDirectory(path, pmode, bRecurse, bExtendByLinks);
		}

		DirStatusCode RemoveDirectory(String &path, bool bRecurse) {
			StartTrace1(System.RemoveDirectory, "dir to remove [" << path << "]");

			// check for empty input parameter
			if (path.Length() == 0) {
				SYSWARNING("empty path given, bailing out!");
				return ePathEmpty;
			}
			// check if directory already exists
			if (!IsDirectory(path) && !IsSymbolicLink(path)) {
				SYSERROR("directory does not exist [" << path << "]");
				return eNotExists;
			}
			// do not allow recursive deletion of absolute paths
			bool bAbsDir = IsAbsolutePath(path);
			if (bRecurse && bAbsDir) {
				SYSERROR("recursive deletion of absolute path not allowed [" << path << "]");
				return eRecurseDeleteNotAllowed;
			}
			return IntRemoveDirectory(path, bRecurse, bAbsDir);
		}

		int GetNumberOfHardLinks(const char *path) {
			StartTrace1(System.GetNumberOfHardLinks, "directory [" << NotNull(path) << "]");
			struct stat64 mystat;

			// acquire inode information
			if (stat64(path, &mystat) == -1) {
				SYSERROR("Could not acquire inode information for " << path << "; stat reports [" << SystemLog::LastSysError()
																	<< "]");
				return -1;
			}
			// return number of hard links to <path>
			return mystat.st_nlink;
		}

#if !defined(WIN32)	 //! TODO:symlinks are supported since vista
		DirStatusCode CreateSymbolicLink(const char *filename, const char *symlinkname) {
			StartTrace1(System.CreateSymbolicLink,
						"directory [" << NotNull(filename) << "] link [" << NotNull(symlinkname) << "]");
			if (symlink(filename, symlinkname) == -1) {
				SYSERROR("Could not create symbolic link " << symlinkname << " to file " << filename << "; symlink reports ["
														   << SystemLog::LastSysError() << "]");
				return eCreateSymlinkFailed;
			}
			// success
			return eSuccess;
		}
#endif

		bool LoadConfigFile(Anything &config, const char *name, const char *ext, String &realfilename) {
			StartTrace(System.LoadConfigFile);
			realfilename = GetFilePath(buildFilename(name, ext));
			std::istream *is = OpenStream(realfilename, (std::ios::in));
			bool result = false;
			if ((is == 0) || !(result = config.Import(*is, realfilename))) {
				String logMsg("cannot import config file ");
				logMsg.Append(realfilename).Append(" from ").Append(name).Append('.').Append(ext);
				SYSERROR(logMsg);
			}
			delete is;
			return result;
		}

		bool LoadConfigFile(Anything &config, const char *name, const char *ext) {
			String dummy;
			return LoadConfigFile(config, name, ext, dummy);
		}

		namespace io {
			int access(const char *path, int amode) {
				StartTrace(System.IO.access);
				return ::access(path, amode);
			}

			int mkdir(const char *path, int pmode) {
				StartTrace1(System.IO.mkdir, "path [" << NotNull(path) << "]");
#if defined(WIN32)
				// no access mode possible for win32
				return ::mkdir(path);
#else
				return ::mkdir(path, pmode);
#endif
			}

			int rmdir(const char *path) {
				StartTrace1(System.IO.rmdir, "path [" << NotNull(path) << "]");
				return ::rmdir(path);
			}

			int unlink(const char *path) {
				StartTrace1(System.IO.unlink, "path [" << NotNull(path) << "]");
				return ::unlink(path);
			}

			int rename(const char *oldfilename, const char *newfilename) {
				StartTrace1(System.IO.rename,
							"oldname [" << NotNull(oldfilename) << "] newname [" << NotNull(newfilename) << "]");
				return ::rename(oldfilename, newfilename);
			}
		}  // namespace io

	}  // namespace system
}  // namespace coast
