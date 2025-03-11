#pragma once

namespace Utilities
{
	namespace EDID
	{
		using _GetFormEditorID = const char* (*)(std::uint32_t);

		/*
		* Given a TESForm, it returns the Editor ID. For forms whose ID is not cached,
		* tries to fetch the ID through PowerOfThree's Tweaks. If this fails, or the form
		* doesn't have an EditorID it returns an empty string.
		* @param a_form Pointer to the form to querry. Must NOT be nullptr.
		* @return The form's EditorID. Empty if the form doesn't have an EditorID or PO3's Tweaks are not installed AND the form's EditorID is not cached.
		*/
		inline std::string GetEditorID(const RE::TESForm* a_form)
		{
			switch (a_form->GetFormType()) {
			case RE::FormType::Keyword:
			case RE::FormType::LocationRefType:
			case RE::FormType::Action:
			case RE::FormType::MenuIcon:
			case RE::FormType::Global:
			case RE::FormType::HeadPart:
			case RE::FormType::Race:
			case RE::FormType::Sound:
			case RE::FormType::Script:
			case RE::FormType::Navigation:
			case RE::FormType::Cell:
			case RE::FormType::WorldSpace:
			case RE::FormType::Land:
			case RE::FormType::NavMesh:
			case RE::FormType::Dialogue:
			case RE::FormType::Quest:
			case RE::FormType::Idle:
			case RE::FormType::AnimatedObject:
			case RE::FormType::ImageAdapter:
			case RE::FormType::VoiceType:
			case RE::FormType::Ragdoll:
			case RE::FormType::DefaultObject:
			case RE::FormType::MusicType:
			case RE::FormType::StoryManagerBranchNode:
			case RE::FormType::StoryManagerQuestNode:
			case RE::FormType::StoryManagerEventNode:
			case RE::FormType::SoundRecord:
				return a_form->GetFormEditorID();
			default:
			{
				static auto tweaks = SKSE::WinAPI::GetModuleHandle(L"po3_Tweaks");
				static auto func = reinterpret_cast<_GetFormEditorID>(SKSE::WinAPI::GetProcAddress(tweaks, "GetFormEditorID"));
				if (func) {
					return func(a_form->formID);
				}
				return {};
			}
			}
		}
	}

	// Makes it easier to make a class a Singleton. EventClass is a Singleton that auto registers as an event class.
	namespace Singleton
	{
		template <class T>
		class ISingleton
		{
		public:
			static T* GetSingleton()
			{
				static T singleton;
				return std::addressof(singleton);
			}

			ISingleton(const ISingleton&) = delete;
			ISingleton(ISingleton&&) = delete;
			ISingleton& operator=(const ISingleton&) = delete;
			ISingleton& operator=(ISingleton&&) = delete;

		protected:
			// Constructor and Destructor are deleted so that ISingleton<T> can NEVER
			// be instantiated, even by accident, and the methods don't show up in stupidsense.
			ISingleton() = default;
			~ISingleton() = default;
		};

		template <class T, class E>
		class EventClass : public ISingleton<T>,
			public RE::BSTEventSink<E>
		{
		public:
			void RegisterListener() {
				auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
				if (!eventHolder) return;

				eventHolder->AddEventSink(this);
			}
		};
	}

	namespace String
	{
		// Functions for dealing with strings. Several common operations are covered.
		// Credit: https://github.com/powerof3/CLibUtil for most of them. 

		/*
		* Splits a string given a delimiter into parts, and returns them as elements in a vector.
		* For example, split("Skyrim.esm|0x0", "|") returns ["Skyrim.esm", "0x0"]
		* @param a_str The string to split.
		* @param a_delimiter The thing to split on, usually a pipe (|).
		* @return A vector of strings.
		*/
		inline std::vector<std::string> split(const std::string& a_str, const std::string& a_delimiter) {
			std::vector<std::string> result;
			size_t start = 0;
			size_t end = a_str.find(a_delimiter);

			while (end != std::string::npos) {
				result.push_back(a_str.substr(start, end - start));
				start = end + a_delimiter.length();
				end = a_str.find(a_delimiter, start);
			}

			result.push_back(a_str.substr(start));
			return result;
		}

		/*
		* Tests a given string to see if it is a hex number.
		* @param a_str The string to test.
		* @param a_requirePrefix If true, the string to test MUST be prefixed with "0x" to be valid.
		* @return True if the string is a hexadecimal.
		*/
		inline bool is_only_hex(std::string_view a_str, bool a_requirePrefix = true)
		{
			if (!a_requirePrefix) {
				return std::ranges::all_of(a_str, [](unsigned char ch) {
					return std::isxdigit(ch);
					});
			}
			else if (a_str.compare(0, 2, "0x") == 0 || a_str.compare(0, 2, "0X") == 0) {
				return a_str.size() > 2 && std::all_of(a_str.begin() + 2, a_str.end(), [](unsigned char ch) {
					return std::isxdigit(ch);
					});
			}
			return false;
		}

		/*
		* Given a string, does its best to convert it to a number. 
		* Given string MUST be convertable to a number, AND must be convertable to the format.
		* @param a_str The string to convert.
		* @param a_hex If true, the string is treated as a hexidecimal number (10 = 17 etc).
		* @return The converted number to <T>.
		* @throws std::invalid_argument If the input string is not a valid number.
		* @throws std::out_of_range If the number is outside the range of type T.
		*/
		template <class T>
		T to_num(const std::string& a_str, bool a_hex = false)
		{
			const int base = a_hex ? 16 : 10;

			if constexpr (std::is_same_v<T, double>) {
				return static_cast<T>(std::stod(a_str, nullptr));
			}
			else if constexpr (std::is_same_v<T, float>) {
				return static_cast<T>(std::stof(a_str, nullptr));
			}
			else if constexpr (std::is_same_v<T, std::int64_t>) {
				return static_cast<T>(std::stol(a_str, nullptr, base));
			}
			else if constexpr (std::is_same_v<T, std::uint64_t>) {
				return static_cast<T>(std::stoull(a_str, nullptr, base));
			}
			else if constexpr (std::is_signed_v<T>) {
				return static_cast<T>(std::stoi(a_str, nullptr, base));
			}
			else {
				return static_cast<T>(std::stoul(a_str, nullptr, base));
			}
		}

		/*
		* Slightly better tolower function. Given a string, converts it to lowercase.
		* @param a_str The string to convert to lowercase.
		* @return The string, in lowercase.
		*/
		inline std::string tolower(std::string_view a_str)
		{
			std::string result(a_str);
			std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
			return result;
		}

		/*
		* Given a string, replaces all instances of a given string with another string.
		* @param a_str The string to perform the search & replace operations on. Modifies the string.
		* @param a_search The sequence to remove.
		* @param a_replace The sequence to replace a_search with.
		* @return True if a replacement has occured.
		*/
		inline bool replace_all(std::string& a_str, std::string_view a_search, std::string_view a_replace)
		{
			if (a_search.empty()) {
				return false;
			}

			std::size_t pos = 0;
			bool wasReplaced = false;
			while ((pos = a_str.find(a_search, pos)) != std::string::npos) {
				a_str.replace(pos, a_search.length(), a_replace);
				pos += a_replace.length();
				wasReplaced = true;
			}

			return wasReplaced;
		}
	}

	namespace Forms
	{
		/*
		* Given a formatted string, attempts to find a form in the game that matches it, casting it to <T*>.
		* Formatted string format: <Modname>.<Extension>|0x<FormID>
		* Example: NotTheBees.esp|0x888
		* @param a_str The formatted string.
		* @return T pointer to the found form, nullptr if not found.
		*/
		template <typename T>
		T* GetFormFromString(const std::string& a_str)
		{
			T* response = nullptr;
			if (const auto splitID = String::split(a_str, "|"); splitID.size() == 2) {
				if (!String::is_only_hex(splitID[0])) return response;
				const auto  formID = String::to_num<RE::FormID>(splitID[0], true);

				const auto& modName = splitID[1];
				if (!RE::TESDataHandler::GetSingleton()->LookupModByName(modName)) return response;

				const auto foundForm = RE::TESDataHandler::GetSingleton()->LookupForm(formID, modName);
				if (foundForm) {
					response = skyrim_cast<T*>(foundForm);
				}
				return response;
			}
			response = RE::TESForm::LookupByEditorID<T>(a_str);
			return response;
		}
	}
}