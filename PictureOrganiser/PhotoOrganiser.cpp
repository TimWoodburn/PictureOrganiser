#include "PhotoOrganiser.h"

//using namespace photoOrganiser;
using namespace std;
using namespace boost::filesystem;
using namespace boost::posix_time;

namespace photoOrganiser
{

	
	const std::string  CPhotoOrganiser::m_months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	const auto widthOfDateString = 12;/*!< The width of the string that is being searched for */
	//date signature offsets :**:** **:**:  where '*' is a wildcard
	const auto dateSigCharOffset_1 = 3;/*!< The offset for the second colon in the date signature string*/
	const auto dateSigCharOffset_2 = 6;/*!< The offset for the space within the date signature string*/
	const auto dateSigCharOffset_3 = 9;/*!< The offset for the third colon in the date signature string*/
	const auto dateSigCharOffset_4 = 12;/*!< The offset for the forth colon in the date signature string*/
	const auto asciiColon = 0x3A;/*!< The ASCII code for a colon ':'*/
	const auto asciiSpace = 0x20;/*!< The ASCII code for a space ' '*/
	const auto asciiNumberOffset = 0x30;/*!< Subtract this number from an ASCII '0' to '9' and the result is the decimal digit value*/
	const auto yearMillenialOffset = 4;/*!< The offset which identifies the 1000th year of the date.  This is subtracted from the position of the first colon in the date signature.*/
	const auto yearCentenialOffset = 3;/*!< The offset which identifies the 100th year of the date.  This is subtracted from the position of the first colon in the date signature.*/
	const auto yearDecadalOffset = 2;/*!< The offset which identifies the 10th year of the date.  This is subtracted from the position of the first colon in the date signature.*/
	const auto yearOffset = 1;/*!< The offset which identifies the least significant year of the date.  This is subtracted from the position of the first colon in the date signature.*/
	const auto searchRange = 5000;/*!< The search length.  This needs to be as short as possible.  The larger this number, the slower the search.  5000 may prevent Olympus Exif dates being missed.*/




	void CFileCopier::operator()()
	{
		const std::string  monthLabels[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		std::cout << "Thread CFileCopier " << std::endl;
		std::string tempDirName = "";
		int copiedFileCounter{ 0 };

		for (auto index = m_startIndex; index < m_endIndex; ++index)
		{

			switch (m_modeRef)
			{
			case BY_YEAR:
				tempDirName = m_targetPathRef.string() + "\\" + std::to_string(m_fileListRef[index].fileYear) + "\\" + m_fileListRef[index].pathToFile.filename().string();
				break;
			case ONE_FOLDER:
				tempDirName = m_targetPathRef.string() + "\\" + m_fileListRef[index].pathToFile.filename().string();
				break;
			case BY_YEAR_MONTH:
				tempDirName = m_targetPathRef.string() + "\\" + std::to_string(m_fileListRef[index].fileYear) + "\\" + monthLabels[m_fileListRef[index].fileMonth] + "_" + std::to_string(m_fileListRef[index].fileYear) + "\\" + m_fileListRef[index].pathToFile.filename().string();
				break;
			default:
				break;
			}
						
			//Copy the file
			copy_file(m_fileListRef[index].pathToFile.string(), tempDirName, copy_option::overwrite_if_exists);
			++copiedFileCounter;

			

			//Reassure the user that the program is still working.
			if ((copiedFileCounter % 1000) == 0)
			{
				cout << "Copied : " << copiedFileCounter << " files, still working." << endl;
			}
		}

		cout << "Operation complete, " << copiedFileCounter << " files copied." << endl;

		m_numOfFilesCopied += copiedFileCounter;


		return;
	}

	CPhotoOrganiser::CPhotoOrganiser() :m_OrganiseMode(BY_YEAR)
	{

	}

	CPhotoOrganiser::~CPhotoOrganiser()
	{

	}

	const std::vector<FileNameDateSet> &CPhotoOrganiser::searchDirectory(const searchMode &searchMode, const boost::filesystem::path &sourcePath)
	{

		std::time_t fileTime = std::time(NULL);
		std::tm *ptm = NULL;

		FileNameDateSet tempFileNameDate;//so we only fetch once
		std::string tempextension;
		bool parsedFile{ false };
		unsigned int rejectedCount{ 0 };



		//Read the source directory recursively, if the file has the correct 
		//extension, store its details in the fileNameDateSetList vector
		try
		{
			if (is_regular_file(sourcePath) || is_directory(sourcePath))    // does p actually exist?
			{
				//Read the directory p recuresively for all the the allowed extensions
				std::cout << "Reading directory - please wait." << endl;
				// Template the iterator in a method>? between recursive_directory_iterator and directory_iterator
				for (recursive_directory_iterator i(sourcePath), end; i != end; ++i)
				{
					//get the last file write time & convert
					fileTime = last_write_time(i->path());
					ptm = std::localtime(&fileTime);

					//fetch the extension once for checking
					tempextension = i->path().extension().string();
					//Only store files in the vector and not paths to directories
					if (is_regular_file(i->path()) == true && (tempextension == ".JPG" |
						tempextension == ".jpg" |
						tempextension == ".MOV" |
						tempextension == ".mp4" |
						tempextension == ".MP4" |
						tempextension == ".avi" |
						tempextension == ".AVI" |
						tempextension == ".bmp" |
						tempextension == ".BMP" |
						tempextension == ".PNG" |
						tempextension == ".png"))
					{

						if (checkExifDate(i->path(), tempFileNameDate) && (tempextension == ".jpg" | tempextension == ".JPG"))
						{
							//call the exif date extractor
							parsedFile = checkExifDate(i->path(), tempFileNameDate);
							m_fileNameDateSetList.push_back(tempFileNameDate);
						}
						else//other file types
						{
							//store the filepath and year in the FileNameDateSet structure
							tempFileNameDate.pathToFile = i->path();
							tempFileNameDate.fileYear = ptm->tm_year + 1900;
							tempFileNameDate.fileMonth = ptm->tm_mon;
							tempFileNameDate.fileDay = ptm->tm_mday;
							m_fileNameDateSetList.push_back(tempFileNameDate);
						}

					}
					else
					{
						++rejectedCount;
						cout << "rejected " << i->path() << " Count :" << rejectedCount << endl;
					}

				}
				//report to the user
				std::wcout << "Size of vector : " << m_fileNameDateSetList.size() << endl;
				std::wcout << "Directories mapped" << endl;
			}//exists(p)
			else
			{
				std::cout << "Cannot find file / directory " << sourcePath << endl;
			}

		}//try
		catch (const filesystem_error& ex)
		{
			std::cout << ex.what() << " exception \n" << endl;
		}

		return m_fileNameDateSetList;
	}

	//Open the file and scan for the date signature and store constructed date in callers fileExifDate
	const bool CPhotoOrganiser::checkExifDate(const boost::filesystem::path &targetFile, FileNameDateSet &fileExifDate)
	{
		bool successStatus{ false };
		std::ifstream file;

		// Open Binary file at the end
		std::ifstream input(targetFile.c_str(), std::ios::ate | std::ios::binary);

		auto end = input.tellg();
		input.seekg(0, std::ios::beg);
		auto beg = input.tellg();
		auto len = end - beg;

		//Date is usually within first 5k bytes
		if (len > searchRange)
		{
			len = searchRange;
		}
		else if (len == 0)//file is empty, exit
			return successStatus;

		// Read in Binary data
		std::vector<char> binaryData(static_cast<unsigned int>(len));
		input.read(&(binaryData[0]), len);

		// Close
		input.close();

		auto vecCapacity = binaryData.size();

		//scan the vector for the date signature as a point of reference
		for (unsigned int z = 0; z < vecCapacity - widthOfDateString; ++z)
		{
			//looking for the date signature :**:** **:**:  where '*' is a wildcard
			//
			if (
				(binaryData[z] == asciiColon)//no ofset, first character
				&& (binaryData[z + dateSigCharOffset_1] == asciiColon)
				&& (binaryData[z + dateSigCharOffset_2] == asciiSpace)
				&& (binaryData[z + dateSigCharOffset_3] == asciiColon)
				&& (binaryData[z + dateSigCharOffset_4] == asciiColon)
				)
			{//then
				//all good, store the date in the callers structure
				fileExifDate.pathToFile = targetFile;
				//construct the year by taking the first ":" as a datum and extracting the previous 4 digits
				fileExifDate.fileYear = ((binaryData[z - yearMillenialOffset] - asciiNumberOffset) * 1000)
					+ ((binaryData[z - yearCentenialOffset] - asciiNumberOffset) * 100)
					+ ((binaryData[z - yearDecadalOffset] - asciiNumberOffset) * 10)
					+ ((binaryData[z - yearOffset] - asciiNumberOffset));

				fileExifDate.fileMonth = ((binaryData[z + 1] - asciiNumberOffset) * 10) + ((binaryData[z + 2] - asciiNumberOffset));
				fileExifDate.fileDay = ((binaryData[z + 4] - asciiNumberOffset) * 10) + ((binaryData[z + 5] - asciiNumberOffset));
				successStatus = true;
			}
		}

		return successStatus;

	}


	const operationStatus CPhotoOrganiser::createTargetDirectories_BY_YEAR(const boost::filesystem::path &targetPath)
	{
		//Now create target directories which will correspond to the years.
		std::wcout << "Creating target directories" << endl;
		create_directory(targetPath);
		std::string tempDirName{ "" };

		//we now have a list of files with their corresponding dates
		//time to extract the years from the vector
		auto iYearContainerLocator = m_fileYears.begin();//ready for the search
		for (auto iElementLocator = m_fileNameDateSetList.begin(); iElementLocator < m_fileNameDateSetList.end(); ++iElementLocator)
		{
			iYearContainerLocator = find(m_fileYears.begin(), m_fileYears.end(), (*iElementLocator).fileYear);
			if (iYearContainerLocator == m_fileYears.end())//year not found, we need to create this directory
			{
				//store the file year in the vector so we only create once
				m_fileYears.push_back((*iElementLocator).fileYear);

				//create the target subdirectory
				tempDirName = targetPath.string() + "\\" + std::to_string((*iElementLocator).fileYear);
				std::cout << tempDirName << endl;
				create_directory(tempDirName);
			}
		}
		return DIRECTORIES_CREATED_OK;
	}

	const operationStatus CPhotoOrganiser::createTargetDirectories_ONE_FOLDER(const boost::filesystem::path &targetPath)
	{
		create_directory(targetPath);
		return DIRECTORIES_CREATED_OK;
	}

	const operationStatus CPhotoOrganiser::createTargetDirectories_BY_YEAR_MONTH(const boost::filesystem::path &targetPath)
	{
		create_directory(targetPath);
		std::string tempDirName{ "" };

		//we now have a list of files with their corresponding dates
		//time to extract the years & months from the vector
		auto iYearContainerLocator = m_fileYears.begin();//ready for the search
		auto iMonthContainerLocator = m_fileMonths.begin();

		//lets iterate through the whole file list
		for (auto iElementLocator = m_fileNameDateSetList.begin(); iElementLocator < m_fileNameDateSetList.end(); ++iElementLocator)
		{
			iYearContainerLocator = find(m_fileYears.begin(), m_fileYears.end(), (*iElementLocator).fileYear);

			if (iYearContainerLocator == m_fileYears.end())//year not found, we need to create this directory, the first one for this year
			{
				//store the file year in the vector so we only create once
				m_fileYears.push_back((*iElementLocator).fileYear);

				//create the target subdirectory
				tempDirName = targetPath.string() + "\\" + std::to_string((*iElementLocator).fileYear);
				std::cout << tempDirName << endl;
				create_directory(tempDirName);//Year

				tempDirName = targetPath.string() + "\\" + std::to_string((*iElementLocator).fileYear) + "\\" + m_months[iElementLocator->fileMonth] + "_" + std::to_string((*iElementLocator).fileYear);
				create_directory(tempDirName);//Month
				//std::cout << tempDirName << endl;
			}
			else if (iYearContainerLocator != m_fileYears.end())//Ok we have see the year before, what about the month?
			{
				tempDirName = targetPath.string() + "\\" + std::to_string((*iElementLocator).fileYear) + "\\" + m_months[iElementLocator->fileMonth] + "_" + std::to_string((*iElementLocator).fileYear);
				create_directory(tempDirName);//Month
				//std::cout << tempDirName << endl;
			}
		}
		return DIRECTORIES_CREATED_OK;
	}

	const operationStatus CPhotoOrganiser::createTargetDirectoryStructure(const organiseMode &mode, const boost::filesystem::path &targetPath)
	{
		std::wcout << "Creating target directories" << endl;

		switch (mode)
		{
		case BY_YEAR:
			createTargetDirectories_BY_YEAR(targetPath);
			break;
		case BY_YEAR_MONTH:
			createTargetDirectories_BY_YEAR_MONTH(targetPath);
			break;
		case ONE_FOLDER:
			createTargetDirectories_ONE_FOLDER(targetPath);
			break;
		default:
			break;
		}



		return operationStatus::DIRECTORIES_CREATED_OK;
	}

	const operationStatus CPhotoOrganiser::copyFiles(const std::vector<FileNameDateSet> &fileList, const boost::filesystem::path &targetPath, const organiseMode &mode, int &numberOfFilesCopied)
	{

		try
		{

			createTargetDirectoryStructure(mode, targetPath);
		
			boost::timer::auto_cpu_timer t;

			boost::ptr_vector<CFileCopier> fileCopierCollection;
			boost::ptr_vector<boost::thread> boostThreadCollection;

			int numOfThreads = 4;//magic number....get rid todo

			int perThread=  fileList.capacity() / numOfThreads;// \todo error check for div 0 or < num of threads
			int lastThreadRemainder = fileList.capacity() - (perThread * numOfThreads);
			int start{ 0 };
			int finish{ 0 };
			
			//determine how many files will be copied by each thread
			//create a thread to pick up it's allocated filenames and paths and copy them to the new structure
			//
			for (int threadIndex = 0; threadIndex < numOfThreads; threadIndex++)
			{
				if (threadIndex < (numOfThreads-1))
				{
					finish += perThread;
				}
				else 
				{
					//special case, make the last thread do the per-thread amount plus the remainder
					finish += perThread;
					finish += lastThreadRemainder;
					cout << "Last thread finish is : " << finish << endl;
				}
					
				cout << "creating thread " << threadIndex << " start = " << start << " finish = " << finish << endl;
				fileCopierCollection.push_back(new CFileCopier(fileList, targetPath, mode, numberOfFilesCopied, start, finish));
				boostThreadCollection.push_back(new boost::thread(fileCopierCollection[threadIndex]));
				
				start += perThread;
			}

			for (int join = 0; join < numOfThreads; join++)
			{
				boostThreadCollection[join].join();
			}

		}//try
		catch (const filesystem_error& ex)
		{
			std::cout << ex.what() << " exception \n" << endl;
		}

		return operationStatus::COPIED_FILES_OK;
	}

}//namespace