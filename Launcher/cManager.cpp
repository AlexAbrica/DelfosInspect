#include "cManager.h"

Launcher::Launcher()
{
	
}
Launcher::~Launcher()
{
	
}

int Launcher::i_CheckPID(QString qstrNameProcess)
{
	string procName = qstrNameProcess.toStdString();
	int pid = -1;

	DIR *dp = opendir("/proc");											// Open the /proc directory
	if (dp != NULL)
	{
		struct dirent *dirp;											// Enumerate all entries in directory until process found
		while (pid < 0 && (dirp = readdir(dp)))
		{
			// Skip non-numeric entries
			int id = atoi(dirp->d_name);
			if (id > 0)
			{
				// Read contents of virtual /proc/{pid}/cmdline file
				string cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
				ifstream cmdFile(cmdPath.c_str());
				string cmdLine;
				getline(cmdFile, cmdLine);
				if (!cmdLine.empty())
				{
					// Keep first cmdline item which contains the program path
					size_t pos = cmdLine.find('\0');
					if (pos != string::npos)
						cmdLine = cmdLine.substr(0, pos);
					// Keep program name only, removing the path
					pos = cmdLine.rfind('/');
					if (pos != string::npos)
						cmdLine = cmdLine.substr(pos + 1);
					// Compare against requested process name
					if (procName == cmdLine)
						pid = id;
				}
			}
		}
	}

	closedir(dp);

	return pid;
}