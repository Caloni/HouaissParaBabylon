#include "Common.h"
#include "Shell.h"

#define BAB_BUILD_TIMEOUT ( 1000 * 60 * 15 )


bool MountIconAndProjPaths(H2BFiles* files)
{
	CString path = files->babInputFile.path;
	int nameOffset = path.ReverseFind(_T('\\'));

	if( nameOffset != -1 )
	{
		path.Delete(nameOffset, path.GetLength());
		files->babIconFile.path = path + _T("\\Houaiss.ico");
		files->babProjFile.path = path + _T("\\Houaiss.gpr");
		files->babOutputFile.path = path + _T("\\Houaiss.bgl");
		return true;
	}

	return false;
}


bool ExtractIconAndProj(H2BFiles* files)
{
	DWORD err;
	bool ret = true;

	err = ExtractResourceToFile(MAKEINTRESOURCE(HOUAISSRAW_ICO), 
		files->babIconFile.path);

	if( err != ERROR_SUCCESS ) 
	{
		ret = false;
		Log("Error %d extracting babylon icon", err);
	}

	err = ExtractResourceToFile(MAKEINTRESOURCE(BABYLONPROJ_GPR), 
		files->babProjFile.path);

	if( err != ERROR_SUCCESS ) 
	{
		ret = false;
		Log("Error %d extracting babylon project", err);
	}

	SetLastError(err);
	return ret;
}


BOOL CreateBuilderProcess(HANDLE job, PCTSTR appName, 
						  PTSTR cmdLine, PCTSTR workDir,
						  H2BOperation& op)
{
	BOOL ret = TRUE;
	DWORD err = ERROR_SUCCESS;
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	if( CreateProcess(appName, cmdLine, NULL, NULL, FALSE, 
		CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB , NULL, workDir, &si, &pi) )
	{
		HANDLE openProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pi.dwProcessId);

		if( openProc )
		{
			if( AssignProcessToJobObject(job, openProc) )
			{
				if( (int) ResumeThread(pi.hThread) != -1 )
				{
					DWORD wait = ERROR_TIMEOUT;
					const DWORD waitStep = 200;

					op.SetStatusMessageDescription(_T("Esse processo pode demorar vários ")
						_T("minutos, dependendo da velocidade de seu computador."));

					for( DWORD i = 0; i * waitStep < BAB_BUILD_TIMEOUT; ++i )
					{
						wait = WaitForSingleObject(openProc, waitStep);

						if( wait == WAIT_OBJECT_0 )
							break;
						else if( op.Cancel() )
						{
							err = ERROR_CANCELLED;
							break;
						}
					}

					if( err == ERROR_CANCELLED )
					{
						Log("Babylon builder process cancellation.");
					}
					else if( wait == WAIT_OBJECT_0 )
					{
						GetExitCodeProcess(openProc, &err);
						Log("Babylon builder process return: %d", err);
					}
					else
					{
						err = ERROR_TIMEOUT;
						Log("Babylon builder timeout");
					}
				}
				else 
				{
					err = GetLastError();
					Log("Error resuming builder process");
				}
			}
			else 
			{
				err = GetLastError();
				Log("Error assigning builder process to job");
				TerminateProcess(openProc, -1);
			}

			CloseHandle(openProc);
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else 
	{
		err = GetLastError();
		Log("Error creating Babylon Builder process");
	}

	if( err != ERROR_SUCCESS )
	{
		SetLastError(err);
		ret = FALSE;
	}

	return ret;
}


BOOL CreateBuilderJob(PCTSTR appName, PTSTR cmdLine, PCTSTR workDir, H2BOperation& op)
{
	BOOL ret = TRUE;
	DWORD err = ERROR_SUCCESS;
	HANDLE job = CreateJobObject(NULL, NULL);

	if( job )
	{
		JOBOBJECT_BASIC_LIMIT_INFORMATION jobLimit = { };

		jobLimit.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		jobLimit.ActiveProcessLimit = 1;

		if( SetInformationJobObject(job, JobObjectBasicLimitInformation, 
			&jobLimit, sizeof(jobLimit)) )
		{
			if( ! CreateBuilderProcess(job, appName, cmdLine, workDir, op) )
				err = GetLastError();
		}

		if( err == ERROR_CANCELLED )
			TerminateJobObject(job, ERROR_CANCELLED);

		CloseHandle(job);
	}

	if( err != ERROR_SUCCESS )
	{
		SetLastError(err);
		ret = FALSE;
	}

	return ret;
}


DWORD BuildDictionary(PCTSTR babylonBuilderPath, H2BFiles* files, H2BOperation& op)
{
	DWORD ret = ERROR_SUCCESS;

	if( MountIconAndProjPaths(files) )
	{
			if( ExtractIconAndProj(files) )
			{
				CString babylonBuilder = babylonBuilderPath;
				CString babylonBuilderArgs;
				CString babylonBuilderCmdLine;

				babylonBuilderArgs.Format(_T("-build \"%s\""),
					files->babProjFile.path);

				babylonBuilderCmdLine = CString(_T("\""))
					+ babylonBuilder + _T("\" ")
					+ babylonBuilderArgs;

				Log("Creating babylon builder process; path: %s; cmdLine: %s",
					(PCTSTR) babylonBuilder, (PCTSTR) babylonBuilderCmdLine);

				if( ! CreateBuilderJob(babylonBuilder, 
					(PTSTR) (PCTSTR) babylonBuilderCmdLine, 
					files->workingDir.path, op) )
				{
					ret = GetLastError();
					Log("Error creating Babylon Builder job");
				}
			}
			else 
			{
				ret = GetLastError();
				Log("Error extracting icon and project files");
		}
	}
	else 
	{
		ret = GetLastError();
		Log("Error mounting icon and project path");
	}

	return ret;
}


DWORD InstallDictionary(H2BFiles* files)
{
	DWORD ret = ERROR_SUCCESS;
	HINSTANCE hi;

	Log("Houaiss-Babylon dictionary: %s", files->babOutputFile.path);

	hi = ShellExecute(NULL, NULL, files->babOutputFile.path, 
		NULL, files->workingDir.path, SW_SHOWDEFAULT);

	if( int(hi) < 32 )
	{
		ret = int(hi);
		Log("Error %d installing dictionary", ret);
	}

	return ret;
}
