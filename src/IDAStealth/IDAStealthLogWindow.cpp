#include "IDACommon.h"
#include "IDAStealthLogWindow.h"

IDAStealthLogWindow::IDAStealthLogWindow()
{
}

// column widths
const int widths[] = { 4, 20, 8 };
// column headers
const char* header[] =
{
	"#",
	"Anti Debugging Measure",
	"Address"
};

ulong idaapi sizer(void* /*obj*/)
{
	msg("returning number of descriptions!!!\n");
	return 1;
}

// function that generates the list line
void idaapi desc(void* /*obj*/, ulong n, char* const* arrptr)
{
	// generate column headers
	if (n == 0)
	{
		for (int i=0; i<qnumber(header); ++i) qstrncpy(arrptr[i], header[i], MAXSTR);
		return;
	}

	--n;
	qsnprintf(arrptr[0], MAXSTR, "%d", n);
	qsnprintf(arrptr[1], MAXSTR, "%s", "NtQueryInformationProcess");
	qsnprintf(arrptr[2], MAXSTR, "%08a", 0x400123);
}

// callback when user presses enter
void idaapi enter_cb(void* /*obj*/, ulong /*n*/)
{
}

//uint32 idaapi destroy(void *obj, uint32 n)
//{
//	return 0;
//}

void IDAStealthLogWindow::show()
{
	choose2(0,									// non-modal window
			-1, -1, -1, -1,						// position is determined by Windows
			NULL,								// pass the created array
			qnumber(header),					// number of columns
			widths,								// widths of columns
			sizer,								// function that returns number of lines
			desc,								// function that generates a line
			"IDAStealth Anti-Debugging Logs",	// window title
			-1,									// use the default icon for the window
			0,									// position the cursor on the first line
			NULL,								// "kill" callback
			NULL,								// "new" callback
			NULL,								// "update" callback
			NULL,								// "edit" callback
			NULL,								// function to call when the user pressed Enter
			NULL,								// function to call when the window is closed
			NULL,								// use default popup menu items
			NULL);								// use the same icon for all lines
}

void IDAStealthLogWindow::addLogEntry(const std::string& /*entry*/)
{
	
}