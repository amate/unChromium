// VersionControl.h
// iniファイルの互換性を保ちます

#pragma once

#define LATESTVERSION	(4)

///////////////////////////////////////////////////////////////////
// CVersionControl

class CVersionControl
{
public:
	// Constrcutor
	//CVersionControl();

	void	Run();

private:

	void	_0to1();
	void	_1to2();
	void	_2to3();
	void	_3to4();
};

































