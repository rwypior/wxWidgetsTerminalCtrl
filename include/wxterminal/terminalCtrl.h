#pragma once

#include <wx/wx.h>

#include <deque>

// Command event

class TerminalCtrl;

class TerminalCommandEvent : public wxCommandEvent
{
public:
    TerminalCommandEvent(wxEventType eventType, TerminalCtrl* ctrl, const wxString& command);
    TerminalCommandEvent* Clone() const;
    const wxString& getCommand();

private:
    const wxString command;
};

// Event sent when user has sent a command
wxDECLARE_EVENT(terminalctrlEVT_COMMAND, TerminalCommandEvent);

// Event sent when internal command processing has been completed
// This might be used to alter prompt
wxDECLARE_EVENT(terminalctrlEVT_POST_COMMAND, TerminalCommandEvent);


// Command queue

template<typename T>
class CommandQueue
{
    using Container = std::deque<T>;
public:
    CommandQueue(size_t limit = 10)
        : containerLimit(limit)
    {
    }

    void push(const T& val)
    {
        // Reject if previous is the same as new
        if (!this->empty() && *std::prev(this->container.end()) == val)
            return;

        if (this->size() >= this->containerLimit)
            this->container.pop_front();
        this->container.push_back(val);
        this->it = this->container.end();
    }

    void clear()
    {
        this->container.clear();
    }

    size_t size() const
    {
        return this->container.size();
    }

    bool empty() const
    {
        return this->size() == 0;
    }

    size_t limit() const
    {
        return this->containerLimit;
    }

    void limit(size_t newLimit)
    {
        this->containerLimit = newLimit;
    }

    const T& current()
    {
        if (this->empty())
            throw std::range_error("Command queue is empty");

        if (this->it == this->container.end())
            return *std::prev(this->container.end());
        return *this->it;
    }

    const T& prev()
    {
        if (this->empty())
            throw std::range_error("Command queue is empty");

        if (this->it != std::prev(this->container.end()))
            this->it++;
        return *this->it;
    }

    const T& next()
    {
        if (this->empty())
            throw std::range_error("Command queue is empty");

        if (this->it != this->container.begin())
            this->it--;
        return *this->it;
    }

private:
    Container container;
    size_t containerLimit;
    typename Container::const_iterator it;
};


// Terminal control

class TerminalCtrl : public wxTextCtrl
{
public:
    bool enableCommandCycling = true;

    TerminalCtrl();
    TerminalCtrl(wxWindow* parent, wxWindowID id,
        const wxString& value = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxASCII_STR(wxTextCtrlNameStr));

    static long createDefaultStyle(long style = 0);

    void setPrompt(const wxString& prompt);
    void setPrompt(const wxString& prompt, const wxString &promptSuffix);
    void setPromptSuffix(const wxString& promptSuffix);

    wxString getInput() const;
    void setCommandQueueLimit(size_t limit);
    void nextCommand();
    void prevCommand();
    bool toggleCommand(int key);

    void clearInput();
    void setInput(const wxString& input);

    bool insertionAtLastLine() const;
    bool insertionAtInput() const;
    void setInsertionAtEnd();
    void setInsertionAtPrompt();

    virtual void AppendText(const wxString& text) wxOVERRIDE;
    virtual void WriteText(const wxString& text) wxOVERRIDE;
    virtual void Clear() wxOVERRIDE;

protected:
    void setup();

    wxPoint getPoint(long position) const;
    wxPoint getLastPoint() const;
    long getPosition(const wxPoint &point) const;

    bool pointLt(const wxPoint& a, const wxPoint& b);
    bool pointLtEquals(const wxPoint& a, const wxPoint& b);

    void insertNewLines();
    void insertPrompt();
    void removePrompt();
    void updatePrompt();

    void removeLastNewLine();

    void onCommand();

private:
    void onChar(wxKeyEvent& event);

private:
    wxString prompt = "";
    wxString promptSuffix = ">";

    wxPoint promptPosition;
    wxPoint promptEndPosition;

    bool noAppendProcessing = false;
    bool noWriteProcessing = false;
    bool newLineRequested = false;
    bool handlingCommand = false;

    CommandQueue<wxString> commands;
};