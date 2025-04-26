#pragma once

namespace Utilities
{
	namespace EDID
	{
		using _GetFormEditorID = const char* (*)(std::uint32_t);

		/// <summary>
		/// Gets the form's EditorID. If PO3's Tweaks are present, also returns EditorIDs that are not normally cached.
		/// </summary>
		/// <param name="a_form">The form to query.</param>
		/// <returns>The form's EditorID as a string. An empty string if the EditorID is not found.</returns>
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
			ISingleton() = default;
			~ISingleton() = default;
		};

		template <class T, class E>
		class EventClass : public ISingleton<T>,
			public RE::BSTEventSink<E>
		{
		public:
			bool RegisterListener() {
				auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
				if (!eventHolder) return false;

				eventHolder->AddEventSink(this);
				return true;
			}
		};
	}

	namespace String
	{
		// Functions for dealing with strings. Several common operations are covered.
		// Credit: https://github.com/powerof3/CLibUtil for most of them. 

		/// <summary>
		/// Splits a string given a delimiter into parts, and returns them as elements in a vector.
		/// </summary>
		/// <param name="a_str">The string to split.</param>
		/// <param name="a_delimiter">The string to split based on, usually a pipe (|).</param>
		/// <returns>A vector of strings. The delimiter is not preserved. For example, split("Skyrim.esm|0x0", "|") returns ["Skyrim.esm", "0x0"].</returns>
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

		/// <summary>
		/// Checks if a given string is a hex number.
		/// </summary>
		/// <param name="a_str">The string to check.</param>
		/// <param name="a_requirePrefix">If true, the number candidate must be prefixed by 0x.</param>
		/// <returns>True if the number is a hexadecimal number, False otherwise.</returns>
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

		/// <summary>
		/// Given a string, does its best to convert it to a number.
		/// </summary>
		/// <typeparam name="T">The type of number to convert it to.</typeparam>
		/// <param name="a_str">The string to convert to a number.</param>
		/// <param name="a_hex">If true, the string is treated as a hexadecimal number.</param>
		/// <returns>A number of type T.</returns>
		/// <throws="std::invalid_argument">If the string is not a number.</throws>
		/// <throws="std::out_of_range">If the string contains a number that is too large to store in T.</throws>
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

		/// <summary>
		/// Given a string, converts it to lowercase.
		/// </summary>
		/// <param name="a_str">The string to convert to lowercase.</param>
		/// <returns>A new string identical to the original string, but lowercase.</returns>
		inline std::string tolower(std::string_view a_str)
		{
			std::string result(a_str);
			std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
			return result;
		}

		/// <summary>
		/// Given a string, replaces all instances of a given substring in it with something else.
		/// </summary>
		/// <param name="a_str">The string to MODIFY. The string is MODIFIED.</param>
		/// <param name="a_search">The substring to replace.</param>
		/// <param name="a_replace">The string to replace the substring with.</param>
		/// <returns>True if replacement happened, False otherwise.</returns>
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

		/// <summary>
		/// Given a string, splits it into a pair of integers. The first integer is always present, the second is optional.
		/// </summary>
		/// <param name="a_str">The string to parse.</param>
		/// <param name="a_delimiter">The delimiter to split the string. If more than one are present, only the first instance is used.</param>
		/// <returns>A pair of ints, where there is always a "first", but the second may not exist.</returns>
		/// <exception cref="std::out_of_range If a given number is too large or too small to fit in an int."></exception>
		/// <exception cref="std::invalid_argument If the given string does not contain a number."></exception>
		template <typename T>
		inline std::enable_if_t<std::is_integral_v<T>,
			std::pair<T, std::optional<T>>> SplitIntegers(const std::string& a_str, const std::string& a_delimiter = ",") {
			auto [min, max] = std::pair<T, std::optional<T>>{ std::numeric_limits<T>::min(), std::nullopt };
			const auto split = Utilities::String::split(a_str, a_delimiter);

			if (split.empty()) {
				throw std::invalid_argument("SplitIntegers: Provided string is empty.");
			}

			bool isHex = Utilities::String::is_only_hex(split[0]);
			min = Utilities::String::to_num<T>(split[0], isHex);
			if (split.size() > 1) {
				isHex = Utilities::String::is_only_hex(split[1]);
				max = Utilities::String::to_num<T>(split[1], isHex);
			}

			return { min, max };
		}

		/// <summary>
		/// Given a string, splits it in 2 parts using a provided delimiter.
		/// </summary>
		/// <param name="a_str">The string to split.</param>
		/// <param name="a_delimiter">The delimiter to split with.</param>
		/// <returns>A pair, where the first part is the string to the left of the delimiter, and the second is the string to the right of the delimiter. If the delimiter is not present, second is nullopt.</returns>
		/// <exception cref="std::invalid_argument">Thrown if the provided string is empty.</exception>
		inline std::pair<std::string, std::optional<std::string>> SplitStrings(const std::string& a_str, const std::string& a_delimiter = ",") {
			const auto split = Utilities::String::split(a_str, a_delimiter);
			if (split.empty()) {
				throw std::invalid_argument("SplitStrings: Provided string is empty.");
			}

			auto [min, max] = std::pair<std::string, std::optional<std::string>>{ a_str, std::nullopt };
			if (split.size() > 1) {
				min = split[0];
				max = split[1];
			}
			return { min, max };
		}
	}

	namespace Forms
	{
		/// <summary>
		/// The standard format I use across all my plugins for parsing strings to get game forms.
		/// Default format: <Modname>.<Extension>|0x<FormID>
		/// Also supports searching by EditorID. Not all forms have their EditorIDs cached by the game, but PO3's Tweaks fixes that.
		/// </summary>
		/// <typeparam name="T">The type to cast the found form as.</typeparam>
		/// <param name="a_str">The formatted string.</param>
		/// <returns>A form of type T*, nullptr if not found.</returns>
		template <typename T>
		T* GetFormFromString(const std::string& a_str)
		{
			T* response = nullptr;
			if (const auto splitID = String::split(a_str, "|"); splitID.size() == 2) {
				const auto& modName = splitID[0];
				if (!RE::TESDataHandler::GetSingleton()->LookupModByName(modName)) {
					return response;
				}
				if (!String::is_only_hex(splitID[1])) {
					return response;
				}

				try {
					const auto  formID = String::to_num<RE::FormID>(splitID[1], true);
					return RE::TESDataHandler::GetSingleton()->LookupForm<T>(formID, modName);
				}
				catch (std::exception& e) {
					return response;
				}
			}
			return RE::TESForm::LookupByEditorID<T>(a_str);
		}
	}
}