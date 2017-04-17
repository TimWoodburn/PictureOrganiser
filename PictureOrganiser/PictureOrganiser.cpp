
#pragma warning(disable: 4996)
#include <iostream>
#include <vector>
#include "PhotoOrganiser.h"




using namespace std;
using namespace photoOrganiser;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: pictureorganiser source_path target_path\n";
		return 1;
	}

	boost::filesystem::path p(argv[1]);   // source directory 
	boost::filesystem::path target(argv[2]);	//target directory
	

	CPhotoOrganiser Photos;
	int i{ 0 };

	const std::vector<FileNameDateSet> fileNameDateSetList = Photos.searchDirectory(RECURSIVE, p);
	Photos.copyFiles(fileNameDateSetList, target, photoOrganiser::BY_YEAR, i);
	cout << "finished, number of files copied = " << i << endl;

	return 0;
}