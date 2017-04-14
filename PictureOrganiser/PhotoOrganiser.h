#pragma once

#pragma warning(disable: 4996)
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "boost\date_time\posix_time\posix_time.hpp"
#include <boost\thread.hpp>
#include <boost/timer/timer.hpp>
#include <boost/ptr_container/ptr_vector.hpp>




namespace photoOrganiser
{
	const enum searchMode
	{
		NON_RECURSIVE,
		RECURSIVE
	};

	const enum organiseMode
	{
		ONE_FOLDER,//done
		BY_YEAR,//done
		BY_YEAR_MONTH,//done
		BY_YEAR_MONTH_DATE,
		FILE_TYPES,//creates directories in the target directory that correspond to the file extension
		MOVIES_ONLY,//mp4, avi, flc,
		PICTURES_ONLY//bmp, jpg, png
	};

	const enum operationStatus
	{
		FAILED,
		DIRECTORIES_CREATED_OK,
		COPIED_FILES_OK
	};

	struct FileNameDateSet
	{
		boost::filesystem::path pathToFile;
		unsigned int fileYear{ 0 };
		unsigned int fileMonth{ 0 };
		unsigned int fileDay{ 0 };
	};

	struct year_month_day
	{
		unsigned int fileYear{ 0 };
		unsigned int fileMonth{ 0 };
		unsigned int fileDay{ 0 };
	};
	/**
	*  A class which will copy files to an organised structure.  Objects of this class are designed to run in boost::thread.

	*/
	class CFileCopier
	{
	public:
		CFileCopier(const std::vector<FileNameDateSet> &fileList, const boost::filesystem::path &targetPath, const organiseMode &mode, int &numberOfFilesCopied, const unsigned int startIndex, const unsigned int endIndex) 
			:m_fileListRef{ fileList },
			m_targetPathRef{ targetPath },
			m_modeRef{ mode },
			m_numOfFilesCopied{ numberOfFilesCopied },
			m_startIndex{startIndex},
			m_endIndex{endIndex}{}
		~CFileCopier(){ std::cout << "destructor called" << std::endl; }

		/**
		* The method which is called to instantiate the file copuier thread.
		*/
		void operator() ();
	private:
		const std::vector<FileNameDateSet> &m_fileListRef;//!< A reference to the caller's file list vector
		const boost::filesystem::path &m_targetPathRef;//!< A reference to the caller's target root path
		const organiseMode &m_modeRef;//!< A reference to the callers mode of operation for the target organisation
		int &m_numOfFilesCopied;//!<  A reference to the caller's local variable which stores the number of files copied
		const unsigned int m_startIndex;//!< Specifies which element will be the first element in the referenced file list vector which will be organised by this thread
		const unsigned int m_endIndex;//!< Specifies which element will be the last element in the referenced file list vector which will be organised by this thread
	};


	/** 
	*  A class which will scan a source directory and copy picture and movie files to a selected structure.
	Attempts to identify the file created date by using Exif metadata.

	*/
	class CPhotoOrganiser
	{
	public:
		/**       
		* Searches a directrory recursively or non-recursively  for media files and returns a list of filenames and paths to the caller.
		* \todo - implement non-recursive searchin CPhotoOrganiser
		* @param searchMode Sets the directory search mode to recursive or non-recursive of type searchMode.       
		* @param sourcePath Sets the source directory from which the search for media files will be based of type boost::filesystem::path.       
		* @return Returns a reference to the vector of found files of type photoOrganiser::fileNameDateSet      
		*/
		const std::vector<FileNameDateSet> &searchDirectory(const searchMode &searchMode, const boost::filesystem::path &sourcePath);
		/**
		* Copies media files to an organised structure.
		*
		* <b>Consider the following example source structure:		</b>\n
		\verbatim
				D:\Pictures											
				D:\Pictures\\Mom										
				D:\Pictures\\Mom\birthday1.jpg						
				D:\Pictures\\Mom\birthday2.jpg						
				D:\Pictures\\Mom\birthday3.mov						
				D:\Pictures\\Dad										
				D:\Pictures\\Dad\party1.bmp							
				D:\Pictures\\Dad\party2.jpg							
				D:\Pictures\\Dad\party3.jpg							
		 \endverbatim \n\n\n

		* <b>photoOrganiser::mode::ONE_FOLDER will organise in the following structure:	</b>\n
		\code photoOrganiser::operationStatus status = Photos.copyFiles(fileNameDateSetList, target, photoOrganiser::ONE_FOLDER, filesCopied);
		\endcode
		\verbatim
				D:\Target											
				D:\Target\birthday1.jpg						
				D:\Target\birthday2.jpg						
				D:\Target\birthday3.mov						
				D:\Target\party1.bmp						
				D:\Target\party2.jpg						
				D:\Target\party3.jpg
		\endverbatim \n\n\n

		* <b>photoOrganiser::mode::BY_YEAR will organise in the following structure:	</b>\n
		\code photoOrganiser::operationStatus status = Photos.copyFiles(fileNameDateSetList, target, photoOrganiser::BY_YEAR, filesCopied);
		\endcode
		\verbatim
				D:\Target											
				D:\Target\2014											
				D:\Target\2014\birthday1.jpg						
				D:\Target\2014\birthday2.jpg						
				D:\Target\2014\birthday3.mov						
				D:\Target\2014\party1.bmp						
				D:\Target\2015										
				D:\Target\2015\party2.jpg							
				D:\Target\2016										
				D:\Target\2016\party3.jpg							
		\endverbatim \n\n\n

		* <b>photoOrganiser::mode::BY_YEAR_MONTH will organise in the following structure:</b>\n
		\code photoOrganiser::operationStatus status = Photos.copyFiles(fileNameDateSetList, target, photoOrganiser::BY_YEAR_MONTH, filesCopied);
		\endcode
		\verbatim
				D:\Target											
				D:\Target\2014										
				D:\Target\2014\Apr									
				D:\Target\2014\Apr\birthday1.jpg					
				D:\Target\2014\Apr\birthday2.jpg					
				D:\Target\2014\Apr\birthday3.mov					
				D:\Target\2014\May\party1.bmp						
				D:\Target\2015										
				D:\Target\2015\May									
				D:\Target\2015\May\party2.jpg						
				D:\Target\2016										
				D:\Target\2016\May									
				D:\Target\2016\May\party3.jpg							
		\endverbatim \n\n\n

		* \todo - Priority 1 copy by file type
		* \todo - Priority 1 copy by year, month, date
		* \todo - Priority 1 copy Movies only
		* \todo - Priority 1 copy pictures only
		* \todo - Priority 3 implement choice of whether to overwrite
		* \todo - Priority 3 supporting the overwrite choice, MD5 scan to report if the file is identical
		* \todo - Priority 3 consider logic to offer facility to rename if filename conflicts are identified with disimilar files.
		* \todo - Priority 2 callbacks to report progress of operations
		* \todo - Priority 1 make the month string lookup via a error checking getter.
		* @param fileList A std::vector of photoOrganiser::FileNameDateSet which contains the list of files to be organised.
		* @param targetPath The boost::filesystem::path path which contains the destination of the organised files.
		* @param mode The mode in which the files are organised (see above)
		* @param numberOfFilesCopied returns the number of files copied
		* @return photoOrganiser::operationStatus to return if the operation was completed successfully.
		*/
		const operationStatus copyFiles(const std::vector<FileNameDateSet> &fileList, const boost::filesystem::path &targetPath, const organiseMode &mode, int &numberOfFilesCopied);
		
		
		//getters
		/**
		* Returns a list of extracted file years 
		* @return const std::vector<unsigned int> a vector of file years so we know which subdirectories to create.
		*/
		const std::vector<unsigned int> &getDirectoriesList(){ return m_fileYears; }
		/**
		* Returns a const reference to the std::vector<FileNameDateSet> list
		* @return Returns a const std::vector<FileNameDateSet> of file paths.
		*/
		const std::vector<FileNameDateSet> &getFileList(){ return m_fileNameDateSetList; }
		 
		//setters
		/**
		* Returns a const reference to the std::vector<FileNameDateSet> list
		* @return Returns a const std::vector<FileNameDateSet> of file paths.
		*/
		void SetOrganiserMode(const organiseMode &mode){ m_OrganiseMode = mode; }
		
		CPhotoOrganiser();
		~CPhotoOrganiser();

	private:
		/**
		* Creates the target directory structure by calling the appropriate handler ready for the file copy.
		* @param mode The mode specifies the organisation of the target directories.
		* @param targetPath The root directory of the target path
		* @return Returns the status of the operation.
		*/
		const operationStatus createTargetDirectoryStructure(const organiseMode &mode, const boost::filesystem::path &targetPath);
		/**
		* Creates a single target directory ready for the file copy.
		* @param targetPath The root directory of the target path
		* @return Returns the status of the operation.
		*/
		const operationStatus createTargetDirectories_ONE_FOLDER(const boost::filesystem::path &targetPath);
		/**
		* Creates a single target directory with subdirectories of the file years ready for the file copy.
		* @param targetPath The root directory of the target path
		* @return Returns the status of the operation.
		*/
		const operationStatus createTargetDirectories_BY_YEAR(const boost::filesystem::path &targetPath);
		/**
		* Creates a single target directory with subdirectories of the file years and then further subdirectories of file months ready for the file copy.
		* @param targetPath The root directory of the target path
		* @return Returns the status of the operation.
		*/
		const operationStatus createTargetDirectories_BY_YEAR_MONTH(const boost::filesystem::path &targetPath);
		/**
		* Creates a single target directory with subdirectories of the file years and then further subdirectories of file months followed by further
		* subdirectories of the date of the month ready for the file copy.  <b>Not yet implemented.<\b>
		* @param targetPath The root directory of the target path
		* @return Returns the status of the operation.
		*/
		const operationStatus createTargetDirectories_BY_YEAR_MONTH_DATE(const boost::filesystem::path &targetPath);
		const operationStatus createTargetDirectories_FILE_TYPES(const boost::filesystem::path &targetPath);
		const operationStatus createTargetDirectories_MOVIES_ONLY(const boost::filesystem::path &targetPath);
		const operationStatus createTargetDirectories_PICTURES_ONLY(const boost::filesystem::path &targetPath);
		const bool checkExifDate(const boost::filesystem::path &targetFile, FileNameDateSet &fileExifDate);


	


		std::vector<unsigned int> m_fileYears;//!< Container for the years found.	
		std::vector<unsigned int> m_fileMonths;//!< Container for the months found.
		std::vector<FileNameDateSet> m_fileNameDateSetList;	//!< Container for the file names and paths
		organiseMode m_OrganiseMode;//!< The mode by which the files will be organised, default set in initialisation list.
		static const std::string  m_months[];//!< Enumerated months of the year, for year/month organisation

	};

}
