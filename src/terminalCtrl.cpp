#include "wxterminal/terminalCtrl.h"

#include <unordered_set>

namespace
{
    std::unordered_set<int> allowedKeys {
        wxKeyCode::WXK_LEFT,
        wxKeyCode::WXK_RIGHT,
        wxKeyCode::WXK_UP,
        wxKeyCode::WXK_DOWN,
        wxKeyCode::WXK_HOME,
        wxKeyCode::WXK_END
    };
}

TerminalCommandEvent::TerminalCommandEvent(wxEventType eventType, TerminalCtrl* ctrl, const wxString& command)
    : wxCommandEvent(eventType, ctrl->GetId())
    , command(command)
{
    this->SetEventObject(ctrl);
}

TerminalCommandEvent* TerminalCommandEvent::Clone() const
{
    return new TerminalCommandEvent(*this);
}

const wxString& TerminalCommandEvent::getCommand()
{
    return this->command;
}

wxDEFINE_EVENT(terminalctrlEVT_COMMAND, TerminalCommandEvent);
wxDEFINE_EVENT(terminalctrlEVT_POST_COMMAND, TerminalCommandEvent);

TerminalCtrl::TerminalCtrl()
    : wxTextCtrl()
{
    this->setup();
}

TerminalCtrl::TerminalCtrl(wxWindow* parent, wxWindowID id,
    const wxString& value,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name)
    : wxTextCtrl(parent, id, value, pos, size, createDefaultStyle(style), validator, name)
{
    this->setup();
}

long TerminalCtrl::createDefaultStyle(long style)
{
    return style | wxTE_MULTILINE | wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB | wxTE_RICH | wxWS_EX_VALIDATE_RECURSIVELY;
}

void TerminalCtrl::setup()
{    
    this->insertPrompt();

    Bind(wxEVT_CHAR, &TerminalCtrl::onChar, this);
    Bind(wxEVT_KEY_DOWN, &TerminalCtrl::onChar, this);
}

void TerminalCtrl::setPrompt(const wxString& prompt)
{
    bool newLineRequested = this->newLineRequested;
    if (this->handlingCommand)
        this->newLineRequested = false;

    wxString command = this->getInput();
    this->clearInput();
    this->prompt = prompt;
    this->updatePrompt();
    this->setInput(command);

    this->newLineRequested = newLineRequested;
}

void TerminalCtrl::setPrompt(const wxString& prompt, const wxString& promptSuffix)
{
    bool newLineRequested = this->newLineRequested;
    if (this->handlingCommand)
        this->newLineRequested = false;

    wxString command = this->getInput();
    this->clearInput();
    this->prompt = prompt;
    this->promptSuffix = promptSuffix;
    this->updatePrompt();
    this->setInput(command);

    this->newLineRequested = newLineRequested;
}

void TerminalCtrl::setPromptSuffix(const wxString& promptSuffix)
{
    bool newLineRequested = this->newLineRequested;
    if (this->handlingCommand)
        this->newLineRequested = false;

    wxString command = this->getInput();
    this->clearInput();
    this->promptSuffix = promptSuffix;
    this->updatePrompt();
    this->setInput(command);

    this->newLineRequested = newLineRequested;
}

wxString TerminalCtrl::getInput() const
{
    long start = this->getPosition(this->promptEndPosition);
    long end = this->getPosition(this->getLastPoint());
    return this->GetRange(start, end);
}

void TerminalCtrl::setCommandQueueLimit(size_t limit)
{
    this->commands.limit(limit);
}

void TerminalCtrl::nextCommand()
{
    if (!this->commands.empty())
    {
        this->setInput(this->commands.next());
    }
}

void TerminalCtrl::prevCommand()
{
    if (!this->commands.empty())
    {
        this->setInput(this->commands.prev());
    }
}

bool TerminalCtrl::toggleCommand(int key)
{
    if (key == wxKeyCode::WXK_UP)
    {
        this->nextCommand();
        return true;
    }
    else if (key == wxKeyCode::WXK_DOWN)
    {
        this->prevCommand();
        return true;
    }

    return false;
}

void TerminalCtrl::clearInput()
{
    long promptEnd = this->getPosition(this->promptEndPosition);
    long end = this->getPosition(this->getLastPoint());
    this->Remove(promptEnd, end);
}

void TerminalCtrl::setInput(const wxString& input)
{
    this->noAppendProcessing = true;
    this->clearInput();
    this->AppendText(input);
    this->noAppendProcessing = false;
    this->setInsertionAtEnd();
}

bool TerminalCtrl::insertionAtLastLine() const
{
    wxPoint insertionPoint = this->getPoint(this->GetInsertionPoint());
    return insertionPoint.y == this->GetNumberOfLines() - 1;
}

bool TerminalCtrl::insertionAtInput() const
{
    wxPoint insertionPoint = this->getPoint(this->GetInsertionPoint());
    return insertionPoint.y == this->promptEndPosition.y && insertionPoint.x >= this->promptEndPosition.x;
}

void TerminalCtrl::setInsertionAtEnd()
{
    long pos = this->getPosition(this->getLastPoint());
    this->DoSetSelection(pos, pos, 0);
}

void TerminalCtrl::setInsertionAtPrompt()
{
    long pos = this->getPosition(this->promptEndPosition);
    this->DoSetSelection(pos, pos, 0);
}

wxPoint TerminalCtrl::getPoint(long position) const
{
    wxPoint res;
    long x, y;
    this->PositionToXY(position, &x, &y);
    res.x = x;
    res.y = y;
    return res;
}

wxPoint TerminalCtrl::getLastPoint() const
{
    return this->getPoint(this->GetValue().length());
}

long TerminalCtrl::getPosition(const wxPoint& point) const
{
    return this->XYToPosition(point.x, point.y);
}

void TerminalCtrl::insertNewLines()
{
    this->noAppendProcessing = true;
    this->AppendText("\n");
    this->noAppendProcessing = false;
}

void TerminalCtrl::insertPrompt()
{
    this->noAppendProcessing = true;
    this->promptPosition = this->getLastPoint();
    this->AppendText(this->prompt + this->promptSuffix);
    this->promptEndPosition = this->getLastPoint();
    this->noAppendProcessing = false;
    
    this->setInsertionAtEnd();
}

void TerminalCtrl::removePrompt()
{
    this->noAppendProcessing = true;
    this->Remove(
        this->getPosition(this->promptPosition),
        this->getPosition(this->promptEndPosition)
    );
    this->noAppendProcessing = false;
}

void TerminalCtrl::updatePrompt()
{
    this->removePrompt();
    this->insertPrompt();
}

void TerminalCtrl::removeLastNewLine()
{
    long length = this->GetValue().length();
    wxString range = this->GetRange(length - 1, length);
    if (range == "\n")
    {
        this->Remove(length - 1, length);
    }
}

void TerminalCtrl::onCommand()
{
    this->handlingCommand = true;

    if (this->GetWindowStyle() & wxTE_RICH)
    {
        this->removeLastNewLine();
    }
    
    this->newLineRequested = true;
    this->noAppendProcessing = true;
    TerminalCommandEvent event(terminalctrlEVT_COMMAND, this, this->getInput());
    ProcessWindowEvent(event);
    this->noAppendProcessing = false;
    this->newLineRequested = false;

    this->insertNewLines();
    this->insertPrompt();
    this->commands.push(event.getCommand());

    this->handlingCommand = false;

    TerminalCommandEvent postevent(terminalctrlEVT_POST_COMMAND, this, this->getInput());
    ProcessWindowEvent(postevent);

    long last = this->getPosition(this->getLastPoint());
    this->DoSetSelection(last, last, 0);
}

bool TerminalCtrl::pointLt(const wxPoint& a, const wxPoint& b)
{
    return a.x < b.x || a.y < b.y;
}

bool TerminalCtrl::pointLtEquals(const wxPoint& a, const wxPoint& b)
{
    return a.x <= b.x || a.y < b.y;
}

void TerminalCtrl::onChar(wxKeyEvent& event)
{
    if (this->enableCommandCycling && this->insertionAtInput() && this->toggleCommand(event.GetKeyCode()))
    {
        return;
    }

    long from, to;
    this->GetSelection(&from, &to);

    if ((event.GetModifiers() & wxMOD_SHIFT) && event.GetKeyCode() == wxKeyCode::WXK_HOME && this->insertionAtLastLine())
    {
        this->DoSetSelection(this->getPosition(this->promptEndPosition), to, 0);
        return;
    }


    if (!(event.GetModifiers() & wxMOD_SHIFT) && event.GetKeyCode() == wxKeyCode::WXK_HOME && this->insertionAtLastLine())
    {
        this->setInsertionAtPrompt();
        return;
    }

    if (allowedKeys.find(event.GetKeyCode()) != allowedKeys.end())
    {
        event.Skip();
        return;
    }

    if ((event.GetModifiers() & wxMOD_CONTROL) && std::tolower(event.GetKeyCode()) == 'c')
    {
        event.Skip();
        return;
    }

    if ((event.GetModifiers() & wxMOD_CONTROL) && std::tolower(event.GetKeyCode()) == 'a')
    {
        this->DoSetSelection(0, this->getPosition(this->getLastPoint()), 0);
        return;
    }

    if (
        (event.GetModifiers() & wxMOD_CONTROL) && 
        (std::tolower(event.GetKeyCode()) == 'v' || std::tolower(event.GetKeyCode()) == 'x') && 
        this->insertionAtInput())
    {
        event.Skip();
        return;
    }

    wxPoint insertionPoint = this->getPoint(this->GetInsertionPoint());

    if (event.GetKeyCode() == wxKeyCode::WXK_RETURN)
    {
        this->onCommand();
        return;
    }

    if (this->pointLt(insertionPoint, this->promptEndPosition))
        return;

    if (event.GetKeyCode() == wxKeyCode::WXK_BACK)
    {
        if (from != to && from >= this->getPosition(this->promptEndPosition))
        {
            event.Skip();
            return;
        }

        if (this->pointLtEquals(insertionPoint, this->promptEndPosition))
            return;
    }

    if (event.GetKeyCode() == wxKeyCode::WXK_DELETE && this->pointLt(insertionPoint, this->promptEndPosition))
        return;

    event.Skip();
}

void TerminalCtrl::AppendText(const wxString& text)
{
    this->noWriteProcessing = true;
    if (this->newLineRequested)
    {
        wxTextCtrl::AppendText("\n");
    }

    long last = this->getPosition(this->getLastPoint());

    if (this->noAppendProcessing)
    {
        this->DoSetSelection(last, last, 0);
        this->WriteText(text);
    }
    else
    {
        int numberOfLines = this->GetNumberOfLines();

        int selectionPoint = this->getPosition(this->promptPosition);   
        int from = selectionPoint;
        int to = selectionPoint;

        wxString prev = this->GetRange(selectionPoint - 1, selectionPoint);
        if (prev == "\n")
            from--;
        this->DoSetSelection(from, to, 0);

        this->WriteText(text + "\n");

        int lineDiff = this->GetNumberOfLines() - numberOfLines;

        this->promptPosition.y += lineDiff;
        this->promptEndPosition.y += lineDiff;
    }
    this->noWriteProcessing = false;

    long pos = this->getPosition(this->promptEndPosition) + 1;
    this->DoSetSelection(pos, pos, 0);
    this->ScrollLines(this->promptEndPosition.y);
}

void TerminalCtrl::WriteText(const wxString& text)
{
    if (this->noWriteProcessing || this->noAppendProcessing)
    {
        wxTextCtrl::WriteText(text);
        return;
    }

    if (this->insertionAtInput())
    {
        wxTextCtrl::WriteText(text);
    }
}

void TerminalCtrl::Clear()
{
    wxTextCtrl::Clear();
    this->insertPrompt();
}