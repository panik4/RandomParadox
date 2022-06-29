#include "CppUnitTest.h"
#include <filesystem>
#include "generic/NameGenerator.h"
#include "generic/ParserUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PdoxTest
{
	auto nameDir = std::filesystem::absolute("../../RandomParadox/resources/names");
	TEST_CLASS(PdoxTest)
	{
	public:
		TEST_METHOD(GenerateTagTwice)
		{
			// load in name rules
			auto nData = Scenario::NameGeneration::prepare(nameDir.string());
			// generate a first tag and check if it is as expected
			const auto tag = Scenario::NameGeneration::generateTag("Abkazhia", nData);
			Assert::AreEqual(tag, std::string{ "ABK" });
			// as the tag is used already, the same input must now produce different output
			const auto unequal = Scenario::NameGeneration::generateTag("Abkazhia", nData);
			Assert::AreNotEqual(unequal, std::string{ "ABK" });
		}
		TEST_METHOD(GenerateAdjective) {
			// TODO
		}
	};
	TEST_CLASS(ParserTest)
	{
	public:
		TEST_METHOD(FormattingInputAsCsv)
		{
			using namespace Scenario::ParserUtils;
			auto result = csvFormat({ "a", "b", "c", "d" }, ';', false);
			std::string abcd = "a;b;c;d\n";
			Assert::AreEqual(result, abcd);
			abcd = "a;b;c;d;\n";
			result = csvFormat({ "a", "b", "c", "d" }, ';', true);
			Assert::AreEqual(result, abcd);
		}
		TEST_METHOD(RemoveCharacterFromString)
		{
			using namespace Scenario::ParserUtils;
			auto str = std::string{ "hello world" };
			removeCharacter(str, 'o');
			Assert::AreEqual(str, { "hell wrld" });
		}
		TEST_METHOD(GetTokensFromString)
		{
			using namespace Scenario::ParserUtils;
			auto tokens = getTokens({ "a;b;c;d;" }, ';');
			std::vector<std::string> referenceTokens = { "a", "b", "c", "d" };
			for (auto i = 0; i < tokens.size(); i++) {
				Assert::AreEqual(tokens[i], referenceTokens[i]);
			}
		}
		TEST_METHOD(GetNumbersFromString)
		{
			using namespace Scenario::ParserUtils;
			auto tokens = getNumbers({ "1;2;3;4;" }, ';');
			std::vector<int> referenceTokens = { 1, 2, 3, 4 };
			for (auto i = 0; i < tokens.size(); i++) {
				Assert::AreEqual(tokens[i], referenceTokens[i]);
			}
		}
		TEST_METHOD(GetNumberBlockFromString)
		{
			using namespace Scenario::ParserUtils;
			auto tokens = getNumberBlock({ "key = {1234 5678}" }, "key");
			std::vector<int> referenceTokens = { 1234, 5678 };
			for (auto i = 0; i < tokens.size(); i++) {
				Assert::AreEqual(tokens[i], referenceTokens[i]);
			}
		}
		TEST_METHOD(ReplaceStringInString)
		{
			using namespace Scenario::ParserUtils;
			std::string content = { "key = {1234 5678}, key = {1234 5678}" };
			replaceOccurences(content, "key", "value");
			Assert::AreEqual(content, { "value = {1234 5678}, value = {1234 5678}" });
		}
		TEST_METHOD(ReplaceLineInString)
		{
			using namespace Scenario::ParserUtils;
			std::string content = { "key1 = {1234 5678}\n key2 = {1234 5678}" };
			replaceLine(content, "key", "valueLine");
			Assert::AreEqual(content, { "valueLine\n key2 = {1234 5678}" });
		}
		TEST_METHOD(FindBracketInString)
		{
			using namespace Scenario::ParserUtils;
			std::string content = { "key1 = {1234 5678}\n key2 = {1234 5678}" };
			int pos = findClosingBracket(content, 0);
			Assert::AreEqual(pos, 17);
			content = { "key1 = {1234 5678\n key2 = {1234 5678" };
			pos = findClosingBracket(content, 0);
			Assert::AreEqual(pos, -1);
		}
		TEST_METHOD(GetBracketInString)
		{
			using namespace Scenario::ParserUtils;
			auto content = getBracketBlock({ "key1 = {1234 5678}\n key2 = {1234 5678}" }, "key2");
			Assert::AreEqual(content, { "key2 = {1234 5678}" });
			auto empty = getBracketBlock({ "key1 = {1234 5678}\n key2 = {1234 5678}" }, "key3");
			Assert::AreEqual(empty, { "" });
		}
		TEST_METHOD(GetBracketContentInString)
		{
			using namespace Scenario::ParserUtils;
			auto content = getBracketBlockContent({ "key1 = {1234 5678}\n key2 = {1234 5678}" }, "key2");
			Assert::AreEqual(content, { "1234 5678" });
			auto empty = getBracketBlockContent({ "key1 = {1234 5678}\n key2 = {1234 5678}" }, "key3");
			Assert::AreEqual(empty, { "" });
		}
	};
}
