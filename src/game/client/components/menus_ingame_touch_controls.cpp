#include <base/color.h>

#include <game/client/components/touch_controls.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/ui_listbox.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

#include "menus.h"

#include <algorithm>

static const char *BEHAVIORS[] = {"Bind", "Bind Toggle", "Predefined"};
static const char *PREDEFINEDS[] = {"Extra Menu", "Joystick Hook", "Joystick Fire", "Joystick Aim", "Joystick Action", "Use Action", "Swap Action", "Spectate", "Emoticon", "Ingame Menu"};
static const char *LABELTYPES[] = {"Plain", "Localized", "Icon"};
static const ColorRGBA LABELCOLORS[2] = { ColorRGBA(0.3f, 0.3f, 0.3f, 1.0f), ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f) };

//This is called when the Touch button editor is rendered, the below one. Used for updating the CLineInput.
void CMenus::OnOpenTouchButtonEditor(bool Force)
{
	//If selected button changes, update the cached information in editor. You can also force changing.
	if(GameClient()->m_TouchControls.m_pLastSelectedButton != nullptr && GameClient()->m_TouchControls.m_pLastSelectedButton == GameClient()->m_TouchControls.m_pSelectedButton && !Force)
	{
		GameClient()->m_TouchControls.m_pLastSelectedButton = GameClient()->m_TouchControls.m_pSelectedButton;
		return;
	}

	if(GameClient()->m_TouchControls.m_pSelectedButton == nullptr)
		dbg_assert(false, "Detected impossible m_pSelectedButton has nullptr value in OnOpenTouchButtonEditor");

	//Reset all cached values.
	m_EditBehaviorType = 0;
	m_PredefinedBehaviorType = 0;
	GameClient()->m_TouchControls.m_CachedNumber = 0;
	m_EditCommandNumber = 0;
	m_InputCommand.Set("");
	m_InputLabel.Set("");
	GameClient()->m_TouchControls.m_vCachedCommands.clear();
	GameClient()->m_TouchControls.m_vCachedCommands.reserve(5);
	m_aCachedVisibilities.fill(2); // 2 means don't have the visibility.

	//These values can't be null. The constructor has been updated. Default:{0,0,50000,50000}, shape = rect.
	m_InputX.Set(std::to_string(GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_X).c_str());
	m_InputY.Set(std::to_string(GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_Y).c_str());
	m_InputW.Set(std::to_string(GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_W).c_str());
	m_InputH.Set(std::to_string(GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_H).c_str());
	m_CachedShape = GameClient()->m_TouchControls.m_pSelectedButton->m_Shape;
	for(const auto &Visibility : GameClient()->m_TouchControls.m_pSelectedButton->m_vVisibilities)
	{
		if((int)Visibility.m_Type >= (int)CTouchControls::EButtonVisibility::NUM_VISIBILITIES)
			dbg_assert(false, "666No acting anymore");
		m_aCachedVisibilities[(int)Visibility.m_Type] = Visibility.m_Parity ? 1 : 0;
	}

	//These are behavior values.
	if(GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior != nullptr)
	{
		std::string BehaviorType = GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior->GetBehaviorType();
		if(BehaviorType == "bind")
		{
			m_EditBehaviorType = 0;
			auto *CastedBehavior = static_cast<CTouchControls::CBindTouchButtonBehavior*>(GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior.get());
			if(CastedBehavior == nullptr)
				dbg_assert(false, "? CastedNULLPTR in bind");
			//Take care m_LabelType must not be null as for now. When adding a new button give it a default value or cry.
			GameClient()->m_TouchControls.m_vCachedCommands.emplace_back(CastedBehavior->GetLabel().m_pLabel, CastedBehavior->GetLabel().m_Type, CastedBehavior->GetCommand().c_str());
			m_InputCommand.Set(CastedBehavior->GetCommand().c_str());
			m_InputLabel.Set(CastedBehavior->GetLabel().m_pLabel);
		}
		else if(BehaviorType == "bind-toggle")
		{
			m_EditBehaviorType = 1;
			auto *CastedBehavior = static_cast<CTouchControls::CBindToggleTouchButtonBehavior*>(GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior.get());
			if(CastedBehavior == nullptr)
				dbg_assert(false, "? CastedNULLPTR in bindtoggle");
			GameClient()->m_TouchControls.m_vCachedCommands = CastedBehavior->GetCommand();
			m_EditCommandNumber = 0;
			if(!GameClient()->m_TouchControls.m_vCachedCommands.empty())
			{
				m_InputCommand.Set(GameClient()->m_TouchControls.m_vCachedCommands[0].m_Command.c_str());
				m_InputLabel.Set(GameClient()->m_TouchControls.m_vCachedCommands[0].m_Label.c_str());
			}
		}
		else if(BehaviorType == "predefined")
		{	
			m_EditBehaviorType = 2;
			const char *PredefinedType = GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior->GetPredefinedType();
			if(PredefinedType == nullptr)
				m_PredefinedBehaviorType = 0;
			else
				for(m_PredefinedBehaviorType = 0; m_PredefinedBehaviorType < 10 && PredefinedType != GameClient()->m_TouchControls.m_BehaviorFactoriesEditor[m_PredefinedBehaviorType].m_pId; m_PredefinedBehaviorType ++);

			if(m_PredefinedBehaviorType == 10)
				dbg_assert(false, "WTF is going on? PredefinedType = %s", PredefinedType);

			if(m_PredefinedBehaviorType == 0)
			{
				auto *CastedBehavior = static_cast<CTouchControls::CExtraMenuTouchButtonBehavior*>(GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior.get());
				if(CastedBehavior == nullptr)
					dbg_assert(false, "? CastedNULLPTR in extramenu");
				GameClient()->m_TouchControls.m_CachedNumber = CastedBehavior->GetNumber();
			}
		}
		else //Empty
		 	dbg_assert(false, "Detected out of bound value in m_EditBehaviorType");
	}
	if(GameClient()->m_TouchControls.m_vCachedCommands.size() < 2)
		GameClient()->m_TouchControls.m_vCachedCommands.resize(2);
	GameClient()->m_TouchControls.m_pLastSelectedButton = GameClient()->m_TouchControls.m_pSelectedButton;
}

void CMenus::InputPosFunction(CLineInputBuffered<7> *Input, std::string *SavedString)
{
    std::string InputValue = (*Input).GetString();
	bool IsDigit = std::all_of(InputValue.begin(), InputValue.end(), [](char Value){
		return std::isdigit(static_cast<unsigned char>(Value));
	});
	if(!IsDigit)
		(*Input).Set(SavedString->c_str());
    auto LeadingZero = std::find_if(InputValue.begin(), InputValue.end(), [](char Value){
        return Value != '0';
    });
    InputValue.erase(InputValue.begin(), LeadingZero);
	*SavedString = (*Input).GetString();
	GameClient()->m_TouchControls.m_ShownRect.value().m_X = std::stoi((*Input).GetString());
	m_UnsavedChanges = true;
}

void CMenus::RenderTouchButtonEditor(CUIRect MainView)
{
	//Update LineInputs and others if Selected button changes.
	OnOpenTouchButtonEditor();
	//Delete if user inputs value that is not digits.
	static std::string s_SavedX = "0", s_SavedY = "0", s_SavedW = "50000", s_SavedH = "50000";

    CUIRect Left, Right, A, B, EditBox, VisRec;
    MainView.VSplitLeft(MainView.w / 4.0f, &Left, &Right);
    Left.Margin(5.0f, &Left);
    Left.HSplitTop(25.0f, &EditBox, &Left);
	Left.HSplitTop(5.0f, nullptr, &Left);
	EditBox.VSplitLeft(25.0f, &A, &EditBox);
	Ui()->DoLabel(&A, "X:", 16.0f, TEXTALIGN_ML);
    if(Ui()->DoClearableEditBox(&m_InputX, &EditBox, 12.0f))
    {
        InputPosFunction(&m_InputX, &s_SavedX);
	}

	//Auto check if the input value contains char that is not digit. If so delete it.
	Left.HSplitTop(25.0f, &EditBox, &Left);
	Left.HSplitTop(5.0f, nullptr, &Left);
	EditBox.VSplitLeft(25.0f, &A, &EditBox);
	Ui()->DoLabel(&A, "Y:", 16.0f, TEXTALIGN_ML);
	if(Ui()->DoClearableEditBox(&m_InputY, &EditBox, 12.0f))
    {
        InputPosFunction(&m_InputY, &s_SavedY);
    }

    Left.HSplitTop(25.0f, &EditBox, &Left);
	Left.HSplitTop(5.0f, nullptr, &Left);
	EditBox.VSplitLeft(25.0f, &A, &EditBox);
	Ui()->DoLabel(&A, "W:", 16.0f, TEXTALIGN_ML);
    if(Ui()->DoClearableEditBox(&m_InputW, &EditBox, 12.0f))
    {
        InputPosFunction(&m_InputW, &s_SavedW);
    }

    Left.HSplitTop(25.0f, &EditBox, &Left);
	Left.HSplitTop(5.0f, nullptr, &Left);
	EditBox.VSplitLeft(25.0f, &A, &EditBox);
	Ui()->DoLabel(&A, "H:", 16.0f, TEXTALIGN_ML);
    if(Ui()->DoClearableEditBox(&m_InputH, &EditBox, 12.0f))
    {
        InputPosFunction(&m_InputH, &s_SavedH);
    }
	
	//Drop down menu for shapes
	Left.HSplitTop(25.0f, &EditBox, &Left);
	Left.HSplitTop(5.0f, nullptr, &Left);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin(5.0f, &A);
	B.VMargin(5.0f, &B);
	Ui()->DoLabel(&A, "Shape:", 16.0f, TEXTALIGN_ML);
	static CUi::SDropDownState s_ButtonShapeDropDownState;
	static CScrollRegion s_ButtonShapeDropDownScrollRegion;
	s_ButtonShapeDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_ButtonShapeDropDownScrollRegion;
	const char* Shapes[] = {"Rect", "Circle"};
	const CTouchControls::EButtonShape NewButtonShape = (CTouchControls::EButtonShape)Ui()->DoDropDown(&B, (int)m_CachedShape, Shapes, std::size(Shapes), s_ButtonShapeDropDownState);
	if(NewButtonShape != m_CachedShape)
	{
		m_CachedShape = NewButtonShape;
		m_UnsavedChanges = true;
	}

	//Right for behaviors, left(center) for visibility. They share 0.75 width of mainview, each 0.375.
	Right.VSplitMid(&VisRec, &Right);
	Right.Margin(5.0f, &Right);
	Right.HSplitTop(25.0f, &EditBox, &Right);
	Right.HSplitTop(5.0f, nullptr, &Right);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin(5.0f, &A);
	B.VMargin(5.0f, &B);
	Ui()->DoLabel(&A, "Behavior Type:", 16.0f, TEXTALIGN_ML);
	static CUi::SDropDownState s_ButtonBehaviorDropDownState;
	static CScrollRegion s_ButtonBehaviorDropDownScrollRegion;
	s_ButtonBehaviorDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_ButtonBehaviorDropDownScrollRegion;
	const int NewButtonBehavior = Ui()->DoDropDown(&B, m_EditBehaviorType, BEHAVIORS, std::size(BEHAVIORS), s_ButtonBehaviorDropDownState);
	if(NewButtonBehavior != m_EditBehaviorType)
	{
		m_EditBehaviorType = NewButtonBehavior;
		if(m_EditBehaviorType == 0)
		{
			m_InputLabel.Set(GameClient()->m_TouchControls.m_vCachedCommands[0].m_Label.c_str());
			m_InputCommand.Set(GameClient()->m_TouchControls.m_vCachedCommands[0].m_Command.c_str());
		}
		if(m_EditBehaviorType == 1)
		{
			if(GameClient()->m_TouchControls.m_vCachedCommands.size() <= static_cast<size_t>(m_EditCommandNumber))
			dbg_assert(false, "GameClient()->m_TouchControls.m_vCachedCommands.size < number, in Dropdown behavior choosing space");
			m_InputLabel.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Label.c_str());
			m_InputCommand.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Command.c_str());
		}
		m_UnsavedChanges = true;
	}

	Right.HSplitTop(25.0f, &EditBox, &Right);
	Right.HSplitTop(5.0f, nullptr, &Right);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin(5.0f, &A);
	B.VMargin(5.0f, &B);
	if(m_EditBehaviorType == 0)
	{
		Ui()->DoLabel(&A, "Command:", 16.0f, TEXTALIGN_ML);
		if(Ui()->DoClearableEditBox(&m_InputCommand, &B, 10.0f))
		{
			GameClient()->m_TouchControls.m_vCachedCommands[0].m_Command = m_InputCommand.GetString();
			m_UnsavedChanges = true;
		}
	}
	else if(m_EditBehaviorType == 1)
	{
		Ui()->DoLabel(&A, "Number:", 16.0f, TEXTALIGN_ML);
		// Decrease Button, increase button and delete button share 1/2 width of B, the rest is for number. 1/6, 1/2, 1/6, 1/6.
		B.VSplitLeft(B.w / 6, &A, &B);
		static CButtonContainer s_DecreaseButton;
		if(DoButton_Menu(&s_DecreaseButton, "-", 0, &A))
		{
			if(m_EditCommandNumber > 0)
			{
				m_EditCommandNumber --;
			}
			if(GameClient()->m_TouchControls.m_vCachedCommands.size() <= static_cast<size_t>(m_EditCommandNumber))
			{
				dbg_assert(false, "commands.size < number at do decrease button");
			}
			m_InputCommand.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Command.c_str());
			m_InputLabel.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Label.c_str());
		}
		B.VSplitLeft(B.w * 0.6f, &A, &B);
		//m_EditCommandNumber counts from 0. But shown from 1.
		Ui()->DoLabel(&A, std::to_string(m_EditCommandNumber + 1).c_str(), 16.0f, TEXTALIGN_MC);
		B.VSplitLeft(B.w / 2.0f, &A, &B);
		static CButtonContainer s_IncreaseButton;
		if(DoButton_Menu(&s_IncreaseButton, "+", 0, &A))
		{
			m_EditCommandNumber ++;
			if((int)GameClient()->m_TouchControls.m_vCachedCommands.size() < m_EditCommandNumber + 1)
			{
				GameClient()->m_TouchControls.m_vCachedCommands.emplace_back("", CTouchControls::CButtonLabel::EType::PLAIN, "");
				m_UnsavedChanges = true;
			}
			if(GameClient()->m_TouchControls.m_vCachedCommands.size() <= static_cast<size_t>(m_EditCommandNumber))
			dbg_assert(false, "commands.size < number at do increase button");
			m_InputCommand.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Command.c_str());
			m_InputLabel.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Label.c_str());
		}
		static CButtonContainer s_DeleteButton;
		if(DoButton_Menu(&s_DeleteButton, FontIcons::FONT_ICON_TRASH, 0, &B))
		{
			const auto DeleteIt = GameClient()->m_TouchControls.m_vCachedCommands.begin() + m_EditCommandNumber;
			GameClient()->m_TouchControls.m_vCachedCommands.erase(DeleteIt);
			if(m_EditCommandNumber + 1 > (int)GameClient()->m_TouchControls.m_vCachedCommands.size())
			{
				m_EditCommandNumber --;
				if(m_EditCommandNumber < 0)
				dbg_assert(false, "Detected m_EditCommandNumber < 0.");
			}
			while(GameClient()->m_TouchControls.m_vCachedCommands.size() < 2)
				GameClient()->m_TouchControls.m_vCachedCommands.emplace_back("", CTouchControls::CButtonLabel::EType::PLAIN, "");
			m_InputCommand.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Command.c_str());
			m_InputLabel.Set(GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Label.c_str());
			m_UnsavedChanges = true;
		}
	}
	else if(m_EditBehaviorType == 2)
	{
		Ui()->DoLabel(&A, "Type:", 16.0f, TEXTALIGN_ML);
		static CUi::SDropDownState s_ButtonPredefinedDropDownState;
		static CScrollRegion s_ButtonPredefinedDropDownScrollRegion;
		s_ButtonPredefinedDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_ButtonPredefinedDropDownScrollRegion;
		const int NewPredefined = Ui()->DoDropDown(&B, m_PredefinedBehaviorType, PREDEFINEDS, std::size(PREDEFINEDS), s_ButtonPredefinedDropDownState);
		if(NewPredefined != m_PredefinedBehaviorType)
		{
			m_PredefinedBehaviorType = NewPredefined;
			m_UnsavedChanges = true;
		}
	}
	Right.HSplitTop(25.0f, &EditBox, &Right);
	Right.HSplitTop(5.0f, nullptr, &Right);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin(5.0f, &A);
	B.VMargin(5.0f, &B);
	if(m_EditBehaviorType == 0)
	{
		Ui()->DoLabel(&A, "Label:", 16.0f, TEXTALIGN_ML);
		if(Ui()->DoClearableEditBox(&m_InputLabel, &B, 10.0f))
		{
			GameClient()->m_TouchControls.m_vCachedCommands[0].m_Label = m_InputLabel.GetString();
			m_UnsavedChanges = true;
		}
	}
	else if(m_EditBehaviorType == 1)
	{
		Ui()->DoLabel(&A, "Command:", 16.0f, TEXTALIGN_ML);
		if(Ui()->DoClearableEditBox(&m_InputCommand, &B, 10.0f))
		{
			GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Command = m_InputCommand.GetString();
			m_UnsavedChanges = true;
		}
	}
	else if(m_EditBehaviorType == 2 && m_PredefinedBehaviorType == 0) // Extra menu type, needs to input number.
	{
		//Increase & Decrease button share 1/2 width, the rest is for label.
		EditBox.VSplitLeft(EditBox.w / 4, &A, &B);
		SMenuButtonProperties Props;
		Props.m_UseIconFont = true;
		static CButtonContainer s_ExtraMenuDecreaseButton;
		if(DoButton_Menu(&s_ExtraMenuDecreaseButton, "-", 0, &A))
		{
			if(GameClient()->m_TouchControls.m_CachedNumber > 0)
			{
				// Menu Number also counts from 1, but written as 0.
				GameClient()->m_TouchControls.m_CachedNumber --;
				m_UnsavedChanges = true;
			}
		}

		B.VSplitLeft(B.w * 2 / 3.0f, &A, &B);
		Ui()->DoLabel(&A, std::to_string(GameClient()->m_TouchControls.m_CachedNumber + 1).c_str(), 16.0f, TEXTALIGN_MC);
		static CButtonContainer s_ExtraMenuIncreaseButton;
		if(DoButton_Menu(&s_ExtraMenuIncreaseButton, "+", 0, &B))
		{
			if(GameClient()->m_TouchControls.m_CachedNumber < 4)
			{
				GameClient()->m_TouchControls.m_CachedNumber ++;
				m_UnsavedChanges = true;
			}
		}
	}
	Right.HSplitTop(25.0f, &EditBox, &Right);
	Right.HSplitTop(5.0f, nullptr, &Right);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin(5.0f, &A);
	B.VMargin(5.0f, &B);
	if(m_EditBehaviorType == 0)
	{
		Ui()->DoLabel(&A, "Label type:", 16.0f, TEXTALIGN_ML);
		static CUi::SDropDownState s_ButtonLabelTypeDropDownState;
		static CScrollRegion s_ButtonLabelTypeDropDownScrollRegion;
		s_ButtonLabelTypeDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_ButtonLabelTypeDropDownScrollRegion;
		const CTouchControls::CButtonLabel::EType NewButtonLabelType = (CTouchControls::CButtonLabel::EType)Ui()->DoDropDown(&B, (int)GameClient()->m_TouchControls.m_vCachedCommands[0].m_LabelType, LABELTYPES, std::size(LABELTYPES), s_ButtonLabelTypeDropDownState);
		if(NewButtonLabelType != GameClient()->m_TouchControls.m_vCachedCommands[0].m_LabelType)
		{
			GameClient()->m_TouchControls.m_vCachedCommands[0].m_LabelType = NewButtonLabelType;
			m_UnsavedChanges = true;
		}
	}
	else if(m_EditBehaviorType == 1)
	{
		Ui()->DoLabel(&A, "Label:", 16.0f, TEXTALIGN_ML);
		if(Ui()->DoClearableEditBox(&m_InputLabel, &B, 10.0f))
		{
			GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_Label = m_InputLabel.GetString();
			m_UnsavedChanges = true;
		}
	}
	Right.HSplitTop(25.0f, &EditBox, &Right);
	Right.HSplitTop(5.0f, nullptr, &Right);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin(5.0f, &A);
	B.VMargin(5.0f, &B);
	if(m_EditBehaviorType == 1)
	{
		Ui()->DoLabel(&A, "Label type:", 16.0f, TEXTALIGN_ML);
		static CUi::SDropDownState s_ButtonLabelTypeDropDownState;
		static CScrollRegion s_ButtonLabelTypeDropDownScrollRegion;
		s_ButtonLabelTypeDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_ButtonLabelTypeDropDownScrollRegion;
		const CTouchControls::CButtonLabel::EType NewButtonLabelType = (CTouchControls::CButtonLabel::EType)Ui()->DoDropDown(&B, (int)GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_LabelType, LABELTYPES, std::size(LABELTYPES), s_ButtonLabelTypeDropDownState);
		if(NewButtonLabelType != GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_LabelType)
		{
			GameClient()->m_TouchControls.m_vCachedCommands[m_EditCommandNumber].m_LabelType = NewButtonLabelType;
			m_UnsavedChanges = true;
		}
	}

	//Visibilities time. This is button's visibility, not virtual.
	VisRec.h = 150.0f;
	VisRec.Margin(5.0f, &VisRec);
	static CScrollRegion s_VisibilityScrollRegion;
	vec2 ScrollOffset(0.0f, 0.0f);
	VisRec.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, 0.25f), IGraphics::CORNER_ALL, 5.0f);
	s_VisibilityScrollRegion.Begin(&VisRec, &ScrollOffset);
	VisRec.y += ScrollOffset.y;
	const std::array<const char*, (size_t)CTouchControls::EButtonVisibility::NUM_VISIBILITIES> VisibilityStrings = {"Ingame", "Zoom Allowed", "Vote Active", "Dummy Allowed", "Dummy Connected", "Rcon Authed",
	"Demo Player", "Extra Menu 1", "Extra Menu 2", "Extra Menu 3", "Extra Menu 4", "Extra Menu 5"};
	for(unsigned Current = 0; Current < (unsigned)CTouchControls::EButtonVisibility::NUM_VISIBILITIES; ++ Current)
	{
		VisRec.HSplitTop(30.0f, &EditBox, &VisRec);
		if(s_VisibilityScrollRegion.AddRect(EditBox))
		{
			EditBox.HSplitTop(5.0f, nullptr, &EditBox);
			if(Ui()->DoButtonLogic(&m_aButtonVisibilityIds[Current], 0, &EditBox, BUTTONFLAG_LEFT))
			{
				m_aCachedVisibilities[Current] += 2;
				m_aCachedVisibilities[Current] %= 3;
				m_UnsavedChanges = true;
			}
			TextRender()->TextColor(LABELCOLORS[m_aCachedVisibilities[Current] == 2 ? 0 : 1]);
			char aBuf[20];
			str_format(aBuf, sizeof(aBuf), "%s%s", m_aCachedVisibilities[Current] == 0 ? "-" : "+", VisibilityStrings[Current]);
			Ui()->DoLabel(&EditBox, aBuf, 16.0f, TEXTALIGN_MC);
			TextRender()->TextColor(TextRender()->DefaultTextColor());
		}
	}
	s_VisibilityScrollRegion.End();


	//Combine left and right together.
	Left.w = MainView.w;
	Left.w -= 10.0f;
	Left.HSplitTop(25.0f, &EditBox, &Left);
	Left.HSplitTop(5.0f, nullptr, &Left);
	//Confirm && Cancel button share 1/2 width, and they will be shaped into square, placed at the middle of their space.
	EditBox.VSplitLeft(EditBox.w / 4.0f, &A, &EditBox);
	A.VMargin((A.w - 100.0f) / 2.0f, &A);
	static CButtonContainer s_ConfirmButton;
	if(DoButton_Menu(&s_ConfirmButton, "Save", 0, &A))
	{
		//Save the cached config to the selected button.
		if(GameClient()->m_TouchControls.m_pSelectedButton == nullptr)
			dbg_assert(false, "nullptr detected in SelectedButton in Save button");
		GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_X = std::stoi(m_InputX.GetString());
		GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_Y = std::stoi(m_InputY.GetString());
		GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_W = std::stoi(m_InputW.GetString());
		GameClient()->m_TouchControls.m_pSelectedButton->m_UnitRect.m_H = std::stoi(m_InputH.GetString());
		GameClient()->m_TouchControls.m_pSelectedButton->m_vVisibilities.clear();
		for(unsigned Iterator = (unsigned)CTouchControls::EButtonVisibility::INGAME; Iterator < (unsigned)CTouchControls::EButtonVisibility::NUM_VISIBILITIES; ++ Iterator)
		{
			if(m_aCachedVisibilities[Iterator] != 2)
				GameClient()->m_TouchControls.m_pSelectedButton->m_vVisibilities.emplace_back((CTouchControls::EButtonVisibility)Iterator, static_cast<bool>(m_aCachedVisibilities[Iterator]));
		}
		GameClient()->m_TouchControls.m_pSelectedButton->UpdateScreenFromUnitRect();
		GameClient()->m_TouchControls.m_pSelectedButton->m_Shape = m_CachedShape;
		if(m_EditBehaviorType == 0)
		{
			GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior = std::make_unique<CTouchControls::CBindTouchButtonBehavior>(GameClient()->m_TouchControls.m_vCachedCommands[0].m_Label.c_str(), GameClient()->m_TouchControls.m_vCachedCommands[0].m_LabelType, GameClient()->m_TouchControls.m_vCachedCommands[0].m_Command.c_str());
		}
		else if(m_EditBehaviorType == 1)
		{
			std::vector<CTouchControls::CBindToggleTouchButtonBehavior::CCommand> vMovingBehavior = GameClient()->m_TouchControls.m_vCachedCommands;
			GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior = std::make_unique<CTouchControls::CBindToggleTouchButtonBehavior>(std::move(vMovingBehavior));
		}
		else if(m_EditBehaviorType == 2)
		{
			if(m_PredefinedBehaviorType != 0)
				GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior = GameClient()->m_TouchControls.m_BehaviorFactoriesEditor[m_PredefinedBehaviorType].m_Factory();
			else
				GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior = std::make_unique<CTouchControls::CExtraMenuTouchButtonBehavior>(GameClient()->m_TouchControls.m_CachedNumber);
		}
		GameClient()->m_TouchControls.m_pSelectedButton->UpdatePointers();
		m_UnsavedChanges = false;
		GameClient()->m_TouchControls.m_pCachedBehavior = GameClient()->m_TouchControls.m_pSelectedButton->m_pBehavior.get();
	}

	EditBox.VSplitLeft(EditBox.w * 2.0f / 3.0f, &A, &B);
	if(m_UnsavedChanges)
	{
		TextRender()->TextColor(ColorRGBA(1.0f, 0.0f, 0.0f, 1.0f));
		Ui()->DoLabel(&A, Localize("Unsaved changes"), 10.0f, TEXTALIGN_MC);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
	}
	B.VMargin((B.w - 100.0f) / 2.0f, &B);
	static CButtonContainer s_CancelButton;
	if(DoButton_Menu(&s_CancelButton, "Cancel", 0, &B))
	{
		//Since the settings are cancelled, reset the cached settings to m_pSelectedButton though selected button didn't change.
		OnOpenTouchButtonEditor(true);
		m_UnsavedChanges = false;
	}

	Left.HSplitTop(25.0f, &EditBox, &Left);
	EditBox.VSplitLeft(EditBox.w / 2.0f, &A, &B);
	A.VMargin((A.w - 150.0f) / 2.0f, &A);
	B.VMargin((B.w - 150.0f) / 2.0f, &B);
	static CButtonContainer s_AddNewButton;
	if(DoButton_Menu(&s_AddNewButton, "New Button", 0, &A))
	{
		GameClient()->m_TouchControls.NewButton();
	}
	static CButtonContainer s_RemoveButton;
	if(DoButton_Menu(&s_RemoveButton, "Delete Button", 0, &B))
	{
		GameClient()->m_TouchControls.DeleteButton();
	}
}

void CMenus::RenderVirtualVisibilityEditor(CUIRect MainView)
{
	CUIRect EditBox;
	const std::array<const ColorRGBA, 2> LabelColor = { ColorRGBA(0.3f, 0.3f, 0.3f, 1.0f), ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f) };
	CUIRect Label;
	MainView.Margin(10.0f, &MainView);
	MainView.HSplitTop(25.0f, &Label, &MainView);
	MainView.HMargin(5.0f, &MainView);
	Ui()->DoLabel(&Label, Localize("Edit Visibilities"), 20.0f, TEXTALIGN_MC);
	const std::array<const char*, (size_t)CTouchControls::EButtonVisibility::NUM_VISIBILITIES> VisibilityStrings = {"Ingame", "Zoom Allowed", "Vote Active", "Dummy Allowed", "Dummy Connected", "Rcon Authed",
		"Demo Player", "Extra Menu 1", "Extra Menu 2", "Extra Menu 3", "Extra Menu 4", "Extra Menu 5"};
	static CScrollRegion s_VirtualVisibilityScrollRegion;
	vec2 ScrollOffset(0.0f, 0.0f);
	MainView.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, 0.25f), IGraphics::CORNER_ALL, 5.0f);
	s_VirtualVisibilityScrollRegion.Begin(&MainView, &ScrollOffset);
	MainView.y += ScrollOffset.y;
	for(unsigned Current = 0; Current < (unsigned)CTouchControls::EButtonVisibility::NUM_VISIBILITIES; ++ Current)
	{
		MainView.HSplitTop(30.0f, &EditBox, &MainView);
		if(s_VirtualVisibilityScrollRegion.AddRect(EditBox))
		{
			EditBox.HSplitTop(5.0f, nullptr, &EditBox);
			if(Ui()->DoButtonLogic(&m_aVisibilityIds[Current], 0, &EditBox, BUTTONFLAG_LEFT))
			{
				GameClient()->m_TouchControls.m_aVirtualVisibilities[Current] = !GameClient()->m_TouchControls.m_aVirtualVisibilities[Current];
			}
			TextRender()->TextColor(LabelColor[GameClient()->m_TouchControls.m_aVirtualVisibilities[Current] ? 1 : 0]);
			Ui()->DoLabel(&EditBox, VisibilityStrings[Current], 16.0f, TEXTALIGN_MC);
			TextRender()->TextColor(TextRender()->DefaultTextColor());
		}
	}
	s_VirtualVisibilityScrollRegion.End();
}