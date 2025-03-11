## CLib Helper
Clib Helper is an assortment of functions that I regularly use across my plugins. Pairs very nicely with [CLib-Utils](https://github.com/powerof3/CLibUtil) by PowerOfThree, and almost all of the string functions are copied from there. 
## Usage
Run these git commands in your project:
```git
git submodule add https://github.com/SeaSparrowOG/clib-helper External/CLibHelper
git submodule update --init --recursive
```
Add these lines in your CMakeLists.txt:
```cmake
add_subdirectory(External/CLibHelper)
include_directories(External/CLibHelper)
```
Then you can use the header normally:
```cpp
#include <Include/CLibHelper/CLibHelper.h>
```
## Some commands
- Utilities::EDID::GetEditorID(RE::TESForm* a_form)
Returns the form's Editor ID. For runtime cached forms, such as keywords, simply runs GetFormEditorID(). For other forms, attempts to retrieve the EDID through PO3's Tweaks. If it is not present, returns an emtpy string. Should be used instead of GetFormEditorID.
- Utilities::Forms::GetFormFromString(const std::string& a_str)
Given a template (`RE::TESForm`, `RE::TESObjectWEAP`, etc) Returns the form that the given string represents cast as a pointer to the template. String format is CoolMod.esp|0x800, but can also accept EditorIDs such as `IronSword`.
- Utilities::Singleton
Contains 2 classes - ISingleton and EventClass. Classes that inherit from ISingleton will be Singleton classes, and classes that inherit from EventClass will be Singleton classes that support an event. EventClass also gives you access to a RegisterListener method, to speed things up. Example usage:
```cpp
// Some header
class MyCoolClass : public EventClass<MyCoolClass, RE::TESMagicEffectApplyEvent>
{
private:
  RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*) override;
}

// Some function
MyCoolClass::GetSingleton()->RegisterListener(); //Now, MyCoolClass receives TESMagicEffectApply events.
```
