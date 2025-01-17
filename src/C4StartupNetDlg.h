/*
 * LegacyClonk
 *
 * Copyright (c) RedWolf Design
 * Copyright (c) 2006, Sven2
 * Copyright (c) 2017-2020, The LegacyClonk Team and contributors
 *
 * Distributed under the terms of the ISC license; see accompanying file
 * "COPYING" for details.
 *
 * "Clonk" is a registered trademark of Matthes Bender, used with permission.
 * See accompanying file "TRADEMARK" for details.
 *
 * To redistribute this file separately, substitute the full license texts
 * for the above references.
 */

// Startup screen for non-parameterized engine start: Network game selection dialog

#pragma once

#include "C4Startup.h"
#include "C4Network2Discover.h"
#include "C4Network2Reference.h"
#include "C4UpdateDlg.h"

const int C4NetMasterServerQueryInterval = 30; // seconds after last and beforenew game query to master server
const int C4NetRefRequestTimeout = 12; // seconds after which the reference request is interrupted
const int C4NetReferenceTimeout = 42; // seconds after which references are removed from the list (C4NetRefRequestTimeout + C4NetMasterServerQueryInterval)
const int C4NetErrorRefTimeout = 10; // seconds after which failed reference requestsare removed
const int C4NetGameDiscoveryInterval = 30; // seconds
const int C4NetMinRefreshInterval = 1; // seconds; minimum time between refreshes

class C4StartupNetListEntry : public C4GUI::Window
{
public:
	C4StartupNetListEntry(C4GUI::ListBox *pForListBox, C4GUI::Element *pInsertBefore, class C4StartupNetDlg *pNetDlg);
	~C4StartupNetListEntry();

	enum QueryType // where the game ref is coming from
	{
		NRQT_Unknown,       // unknown source
		NRQT_GameDiscovery, // by UDP broadcast in local network
		NRQT_Masterserver,  // by internet masterserver
		NRQT_DirectJoin,    // by user-entered address
	};

	enum TimeoutType
	{
		TT_RefReqWait,   // C4NetMasterServerQueryInterval for masterserver; C4NetErrorRefTimeout for other ref clients
		TT_Reference,    // C4NetReferenceTimeout
		TT_Masterserver, // C4NetMasterServerQueryInterval
		TT_Refresh,      // Refresh as soon as possible
	};

	enum { InfoLabelCount = 5, MaxInfoIconCount = 10, };

private:
	C4StartupNetDlg *pNetDlg;
	C4GUI::ListBox *pList;
	StdStrBuf sRefClientAddress; // set during reference retrieval: reference server address
	C4Network2RefClient *pRefClient; // set during reference retrieval: reference request client
	C4Network2Reference *pRef; // set for retrieved references
	bool fError; // if set, the label was changed to an error message and no more updates are done
	StdStrBuf sError;
	QueryType eQueryType; // valid if pRefClient is set: Where the ref query is originating

	time_t iTimeout; // lifetime: If set, marks time when the item is removed or should re-query
	time_t iRequestTimeout; // if this times out while the request remains busy, the ref client assumes that the tcp socket blocked upon request and aborts it
	int32_t iNumFails; // number of times this reference has retried connecting

	C4GUI::Icon *pIcon; // scenario icon
	C4GUI::Label *pInfoLbl[InfoLabelCount]; // info labels for reference or query
	C4GUI::Icon *pInfoIcons[MaxInfoIconCount]; // right-aligned status icons at topright position
	int32_t iInfoLink; // if !=-1, index of the label that should be turned into a clickable link (for MotD-link)
	int32_t iInfoIconCount;
	int32_t iSortOrder;
	bool fIsSmall;     // set if the item is in collapsed state
	bool fIsCollapsed; // set if the item is forced to collapsed state
	bool fIsEnabled;   // if not set, the item is grayed out
	bool fIsImportant; // if set, the item is presented in yellow (lower priority than fIsEnabled)
	C4Rect rctIconSmall; // bounds for small icon
	C4Rect rctIconLarge; // bounds for large icon

	StdStrBuf sInfoText[InfoLabelCount];

	bool QueryReferences();
	static const char *GetQueryTypeName(QueryType eQueryType); // name of QueryType values
	void SetError(const char *szErrorText, TimeoutType eTimeout); // change secondary label to error label, mark error and set a removal timer
	void SetTimeout(TimeoutType eTimeout);
	C4StartupNetListEntry *AddReference(C4Network2Reference *pAddRef, C4GUI::Element *pInsertBefore); // add a reference list item to the same list
	void InvalidateStatusIcons() { iInfoIconCount = 0; } // schedule all current status icons for removal when UpdateText is called next
	void AddStatusIcon(C4GUI::Icons eIcon, const char *szToolTip); // add a status icon with the specified tooltip

	void UpdateSmallState();
	void UpdateEntrySize();
	void UpdateText(); // strings to labels

protected:
	virtual int32_t GetListItemTopSpacing() override { return fIsCollapsed ? 5 : 10; }
	virtual void DrawElement(C4FacetEx &cgo) override;

	C4GUI::Element *GetNextLower(int32_t sortOrder); // returns the element before which this element should be inserted

public:
	void ClearRef(); // del any ref/refclient/error data
	void SetRefQuery(const char *szAddress, QueryType eQueryType); // start loading references fromt he specified address
	void SetReference(C4Network2Reference *pNewRef); // replace the reference

	bool Execute(); // update stuff - if false is returned, the item is to be removed
	void OnRequestFailed(); // called on connection error or timeout
	bool OnReference(); // like Execute(), but only called if some reference was received
	void UpdateCollapsed(bool fToCollapseValue);

	const char *GetError() { return fError ? sError.getData() : nullptr; } // return error message, if any is set
	C4Network2Reference *GrabReference(); // grab the reference so it won't be deleted when this item is removed
	C4Network2Reference *GetReference() const { return pRef; } // have a look at the reference
	bool IsSameHost(const C4Network2Reference *pRef2); // check whether the reference was created by the same host as this one
	bool IsSameAddress(const C4Network2Reference *pRef2); // check whether there is at least one matching address (address and port)
	bool IsSameRefQueryAddress(const char *szJoinAddress); // check whether reference query was created with same host address
	const char *GetJoinAddress(); // ref queries only: Get direct join address
};

// startup dialog: Network game selection
class C4StartupNetDlg : public C4StartupDlg, private C4InteractiveThread::Callback
{
public:
	C4StartupNetDlg();
	~C4StartupNetDlg();

private:
	enum DlgMode { SNDM_GameList = 0, SNDM_Chat = 1 };
	C4GUI::Tabular *pMainTabular; // main tabular control: Contains game selection list and chat control
	C4GUI::ListBox *pGameSelList; // game selection listbox
	C4KeyBinding *pKeyRefresh, *pKeyBack, *pKeyForward;
	C4GUI::CallbackButton<C4StartupNetDlg, C4GUI::IconButton> *btnGameList, *btnChat; // left side buttons
	C4GUI::CallbackButton<C4StartupNetDlg, C4GUI::IconButton> *btnInternet, *btnRecord; // right side buttons
	C4GUI::Button *btnJoin, *btnRefresh;
	C4GUI::Edit *pJoinAddressEdt;
	class C4ChatControl *pChatCtrl;
	C4GUI::WoodenLabel *pChatTitleLabel;
	C4StartupNetListEntry *pMasterserverClient; // set if masterserver query is enabled: Checks clonk.de for new games
	bool fIsCollapsed; // set if the number of games in the list requires them to be displayed in a condensed format
	bool fUpdatingList; // set during list update - prevent selection update calls

	C4Sec1TimerCallback<C4StartupNetDlg> *pSec1Timer; // engine timer hook for list updates

	C4Network2IODiscoverClient DiscoverClient; // disocver client to search for local hosts

	int iGameDiscoverInterval; // next time until a game discovery is started
	time_t tLastRefresh; // time of last refresh
	bool fIgnoreUpdate; // set after user pressed "No" on a server redirect once

protected:
	virtual bool HasBackground() override { return true; }
	virtual void DrawElement(C4FacetEx &cgo) override;

	virtual C4GUI::Control *GetDefaultControl() override; // get Auto-Focus control
	C4GUI::Control *GetDlgModeFocusControl(); // get control to be focused when main tabular sheet changes

	virtual bool OnEnter() override { DoOK(); return true; }
	virtual bool OnEscape() override { DoBack(); return true; }
	bool KeyBack() { return DoBack(); }
	bool KeyRefresh() { DoRefresh(); return true; }
	bool KeyForward() { DoOK(); return true; }

	virtual void OnShown() override; // callback when shown: Start searching for games
	virtual void OnClosed(bool fOK) override; // callback when dlg got closed: Return to main screen
	void OnBackBtn(C4GUI::Control *btn) { DoBack(); }
	void OnRefreshBtn(C4GUI::Control *btn) { DoRefresh(); }
	void OnCreateGameBtn(C4GUI::Control *btn) { CreateGame(); }
	void OnJoinGameBtn(C4GUI::Control *btn) { DoOK(); }
	void OnSelChange(class C4GUI::Element *pEl) { UpdateSelection(true); }
	void OnSelDblClick(class C4GUI::Element *pEl) { DoOK(); }
	void OnBtnGameList(C4GUI::Control *btn);
	void OnBtnChat(C4GUI::Control *btn);
	void OnBtnInternet(C4GUI::Control *btn);
	void OnBtnRecord(C4GUI::Control *btn);
	void OnBtnUpdate(C4GUI::Control *btn) { C4UpdateDlg::CheckForUpdates(GetScreen()); }

	C4GUI::Edit::InputResult OnJoinAddressEnter(C4GUI::Edit *edt, bool fPasting, bool fPastingMore)
	{
		DoOK(); return C4GUI::Edit::IR_Abort;
	}

	void OnChatTitleChange(const StdStrBuf &sNewTitle);

private:
	static bool IsOpen(C4StartupNetDlg *instance);
	void UpdateMasterserver(); // creates masterserver object if masterserver is enabled; destroy otherwise
	void UpdateList(bool fGotReference = false);
	void UpdateCollapsed();
	void UpdateSelection(bool fUpdateCollapsed);
	void UpdateDlgMode(); // update button visibility after switching between game sel list and chat

	void AddReferenceQuery(const char *szAddress, C4StartupNetListEntry::QueryType eQueryType); // add a ref searcher entry and start searching

	DlgMode GetDlgMode();

	// callback from C4Network2ReferenceClient
	virtual void OnThreadEvent(C4InteractiveEventType eEvent, const std::any &eventData) override;

public:
	bool DoOK(); // join currently selected item
	bool DoBack(); // abort dialog
	void DoRefresh(); // restart network search
	void CreateGame(); // switch to scenario selection in network mode

	void OnReferenceEntryAdd(C4StartupNetListEntry *pEntry);

	void OnSec1Timer(); // idle proc: update list

	void SetIgnoreUpdate(bool to_val) { fIgnoreUpdate = to_val; }
	bool GetIgnoreUpdate() const { return fIgnoreUpdate; }
};
