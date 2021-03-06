#include "MemHelpers.hpp"

byte MemHelpers::getLowestStringTuning() {
	uintptr_t addrTuning = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_tuning, Offsets::ptr_tuningOffsets);

	if (!addrTuning)
		return NULL;

	return (*(Tuning*)addrTuning).strA;
}

bool MemHelpers::IsExtendedRangeSong() {
	uintptr_t addrTimer = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_timer, Offsets::ptr_timerOffsets);

	if (!addrTimer)
		return false;

	byte currentTuning = MemHelpers::getLowestStringTuning();

	if (currentTuning != 0 && currentTuning <= (256 + Settings::GetModSetting("ExtendedRangeMode")))
		return true;
	return false;
}

std::string MemHelpers::GetCurrentMenu(bool GameNotLoaded) {
	if (GameNotLoaded) { // It seems like the third level of the pointer isn't initialized until you reach the UPLAY login screen, but the second level actually is, and in there it keeps either an empty string, "TitleMenu", "MainOverlay" (before you reach the login) or some gibberish that's always the same (after that) 
		uintptr_t preMainMenuAdr = MemUtil::FindDMAAddy(Offsets::ptr_currentMenu, Offsets::ptr_preMainMenuOffsets, GameNotLoaded);

		std::string currentMenu((char*)preMainMenuAdr);

		if (lastMenu == "TitleScreen" && lastMenu != currentMenu)  // I.e. check if its neither one of the possible states
			canGetRealMenu = true;
		else {
			lastMenu = currentMenu;
			return "pre_enter_prompt";
		}
	}

	if (!canGetRealMenu)
		return "pre_enter_prompt";

	uintptr_t currentMenuAdr = MemUtil::FindDMAAddy(Offsets::ptr_currentMenu, Offsets::ptr_currentMenuOffsets, GameNotLoaded); // If game hasn't loaded, take the safer, but possibly slower route

	// Thank you for serving the cause comrade, but you aren't needed any more
	//if (currentMenuAdr > 0x30000000) // Ghetto checks for life, if you haven't eneterd the game it usually points to 0x6XXXXXXX and if you try to dereference that, you get yourself a nice crash
	//	return "pre_enter_prompt";

	if (!currentMenuAdr)
		return "where are we actually";

	std::string currentMenu((char*)currentMenuAdr);
	//std::cout << currentMenu << std::endl;
	return currentMenu;
}

void MemHelpers::ToggleLoft() {

	// Old Method (Works for most builds but bugged out for some)
	//uintptr_t nearAddr = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_loft, Offsets::ptr_loft_nearOffsets);
	//if (*(float*)nearAddr == 10)
	//	*(float*)nearAddr = 10000;
	//else
	//	*(float*)nearAddr = 10;

	uintptr_t farAddr = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_loft, Offsets::ptr_loft_farOffsets);

	if (*(float*)farAddr == 10000)
		*(float*)farAddr = 1;
	else
		*(float*)farAddr = 10000;
}

std::string MemHelpers::ShowSongTimer() {
	uintptr_t addrTimer = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_timer, Offsets::ptr_timerOffsets);

	if (!addrTimer)
		return "";

	return std::to_string(*(float*)addrTimer);
}


void MemHelpers::ShowCurrentTuning() {
	byte lowestStringTuning = MemHelpers::getLowestStringTuning();
	if (lowestStringTuning == NULL)
		return;

	std::string valStr = std::to_string(lowestStringTuning);
	std::cout << valStr << std::endl;
}

void MemHelpers::ToggleCB(bool enabled) {
	uintptr_t addrTimer = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_timer, Offsets::ptr_timerOffsets);
	uintptr_t cbEnabled = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_colorBlindMode, Offsets::ptr_colorBlindModeOffsets);

	if (!addrTimer)
		return;

	if (*(byte*)cbEnabled != (byte)enabled) //JIC, no need to write the same value constantly
		*(byte*)cbEnabled = enabled;
}

void MemHelpers::PatchCDLCCheck() {
	uint8_t* VerifySignatureOffset = Offsets::cdlcCheckAdr;

	if (VerifySignatureOffset) {
		if (!MemUtil::PatchAdr(VerifySignatureOffset + 8, (UINT*)Offsets::patch_CDLCCheck, 2))
			printf("Failed to patch verify_signature!\n");
		else
			printf("Patch verify_signature success!\n");
	}
}

int* MemHelpers::GetWindowSize() {
	RECT WindowSize;
	if (GetWindowRect(FindWindow(NULL, L"Rocksmith 2014"), &WindowSize))
	{
		int width = WindowSize.right - WindowSize.left;
		int height = WindowSize.bottom - WindowSize.top;

		int* dimensions = new int[2]{ width, height };

		return dimensions;
	}
	else
		return new int[2]{ 0, 0 };
}

bool MemHelpers::IsInStringArray(std::string stringToCheckIfInsideArray, std::string* stringArray, std::vector<std::string> stringVector) {
	if (stringArray != NULL) {
		for (unsigned int i = 0; i < stringArray->length(); i++) {
			if (stringToCheckIfInsideArray == stringArray[i])
				return true;
		}
	}
	else if (stringVector != std::vector<std::string>()) {
		for (unsigned int i = 0; i < stringVector.size(); i++) {
			if (stringToCheckIfInsideArray == stringVector[i])
				return true;
		}
	}

	return false;
}

// textColorHex is Hex for AA,RR,GG,BB or FFFFFFFF (8 F's). If your text doesn't show up, make sure you lead with FF (or 255 in hex).
void MemHelpers::DX9DrawText(std::string textToDraw, int textColorHex, int topLeftX, int topLeftY, int bottomRightX, int bottomRightY, LPDIRECT3DDEVICE9 pDevice)
{
	// If the user changes resolutions, we want to scale the text dynamically. This also covers the first font creation as the font and WindowSize variables are all null to begin with.
	if (WindowSizeX != (MemHelpers::GetWindowSize()[0] / 96) || WindowSizeY != (MemHelpers::GetWindowSize()[1] / 72 || CustomDX9Font == NULL)) {
		WindowSizeX = (MemHelpers::GetWindowSize()[0] / 96);
		WindowSizeY = (MemHelpers::GetWindowSize()[1] / 72);

		CustomDX9Font = D3DXCreateFontA(pDevice, WindowSizeX, WindowSizeY, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &DX9FontEncapsulation); // Create a new font
	}

	// Create Area To Draw Text
	RECT TextRectangle;
	TextRectangle.left = topLeftX, TextRectangle.top = topLeftY, TextRectangle.right = bottomRightX, TextRectangle.bottom = bottomRightY;

	// Preload And Draw The Text (Supposed to reduce the performance hit (It's D3D/DX9 but still good practice))
	DX9FontEncapsulation->PreloadTextA(textToDraw.c_str(), textToDraw.length());
	DX9FontEncapsulation->DrawTextA(NULL, textToDraw.c_str(), -1, &TextRectangle, DT_LEFT | DT_NOCLIP, textColorHex);

	// Let's clean up our junk, since fonts don't do it automatically.
	if (DX9FontEncapsulation) {
		DX9FontEncapsulation->Release();
		DX9FontEncapsulation = NULL;
	}	
}

void MemHelpers::ToggleDrunkMode(bool enable) {
	uintptr_t noLoft = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_loft, Offsets::ptr_loft_farOffsets);

	if (enable) {
		if (*(float*)noLoft == 1) {
			D3DHooks::ToggleOffLoftWhenDoneWithMod = true;
			MemHelpers::ToggleLoft();
		}
	}
	else {
		*(float*)Offsets::ptr_drunkShit = 0.3333333333;

		if (D3DHooks::ToggleOffLoftWhenDoneWithMod) {
			MemHelpers::ToggleLoft();
			D3DHooks::ToggleOffLoftWhenDoneWithMod = false;
		}
	}
}

bool MemHelpers::IsInSong() {
	return IsInStringArray(GetCurrentMenu(), 0, songModes);
}

float MemHelpers::RiffRepeaterSpeed(float newSpeed) {
	uintptr_t riffRepeaterSpeed = MemUtil::FindDMAAddy(Offsets::baseHandle + Offsets::ptr_songSpeed, Offsets::ptr_songSpeedOffsets);
	
	if (newSpeed != NULL) // If we aren't just trying to see the current speed.
		*(float*)riffRepeaterSpeed = newSpeed;
	return *(float*)riffRepeaterSpeed;
}

void MemHelpers::AutomatedOpenRRSpeedAbuse() {
	Sleep(5000); // Main animation from Tuner -> In game where we can control the menu
	std::cout << "Triggering RR" << std::endl;

	Util::SendKey(VK_SPACE); // Open RR Menu
	Sleep(666); // Menu animations do be slow

	Util::SendKey(VK_DOWN); // Difficulty

	Util::SendKey(VK_DOWN); // Speed

	RiffRepeaterSpeed(100.00001f); // Allow us to change speed values. Keep this number as close to 100 as possible to allow NSP to use this number. It doesn't like being reset to 100 at the bottom of this function.

	Util::SendKey(VK_LEFT); // Trigger our new speed requirement

	Util::SendKey(VK_RIGHT); // Reset back to 100% in the UI

	Util::SendKey(VK_DELETE); // Return to the song

	RiffRepeaterSpeed(100.f); // Reset to 100% speed so the end user doesn't experience any speed ups / slow downs when an event isn't triggered.

	automatedSongSpeedInThisSong = true; // Don't run this again in the same song if we put this in a loop.
	useNewSongSpeed = true; // Show RR Speed Text
}