/*
 * @brief: business common define
 */

#if !defined gehua_business_common_define_h_
#define gehua_business_common_define_h_

namespace gehua {

enum BusinessStatus {
	BSBase = 1,
	BSPortal = 2,
	BSGame = 3,
	BSVOD = 4,
	BSPending = 5,
};

enum TerminalClass {
	TerminalSTB = 1,
	TerminalPhone = 2,
	TerminalPC = 3,
};

enum STBClass {
	STBOneWay = 1,
	STBTwoWayHD = 2,
	STBTwoWaySD = 3,
};

enum PhoneClass {
	PhoneAndriod = 1,
	PhoneIPhone = 2,
	PhoneWPhone = 3,
};

enum PCOSClass {
	OSWindows = 1,
	OSLinux = 2,
	OSMac = 3,
};

} // namespace gehua

#endif // !gehua_business_common_define_h_
