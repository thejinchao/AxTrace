/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace AT3
{

/**Config System
*/
class Config
{
	/*************************************************************************
		Public Methods
	*************************************************************************/
public:
	/** Load setting from regist*/
	void loadSetting(void);
	/** Save setting to regist */
	void saveSetting(void) const;
	/** copy other to self*/
	void copyFrom(const Config& other);

	bool getCapture(void) const { return m_bCapture; }
	void setCapture(bool c) { m_bCapture=c; }
	bool getAutoScroll(void) const { return m_bAutoscroll; }
	void setAutoScroll(bool a) { m_bAutoscroll=a; }
	HFONT getFont(void) const { return m_hFont; }
	void setFont(LPLOGFONT lf);

	const std::string& getFilterScript(void) const { return m_filterScript; }

	/*************************************************************************
		Implementation Methods
	*************************************************************************/
private:
	/** Reset setting to default
	*/
	void _resetDefaultSetting(void);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	bool m_bCapture;			//!< Capturing or stop capture	(default : true)
	bool m_bAutoscroll;			//!< Auto scroll trace window	(default : true)
	bool m_bAlwaysOnTop;		//!< Mainframe always on top	(default : false)
	bool m_bShowMilliseconds;	//!< Show milliseconds (default : true)

	HFONT m_hFont;
	std::string m_filterScript;

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	Config();
	Config(const Config& other);
	~Config();
};

}