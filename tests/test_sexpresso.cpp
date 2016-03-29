// Author: Isak Andersson 2016 bitpuffin dot com

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <string>
#include <cstdint>
#include "sexpresso.hpp"

TEST_CASE("Empty string") {
	auto str = std::string{};
	REQUIRE(str.empty());

	auto err = std::string{};
	auto s = sexpresso::parse(str, err);

	REQUIRE(err.empty());
	REQUIRE(s.kind == sexpresso::SexpValueKind::SEXP);
	REQUIRE(s.value.str.empty());
	REQUIRE(s.value.sexp.empty());
	REQUIRE(s.childCount() == 0);
	REQUIRE(s.isNil());
}

TEST_CASE("Empty sexp") {
	auto str = std::string{"()"};

	auto err = std::string{};
	auto s = sexpresso::parse(str, err);

	REQUIRE(err.empty());
	REQUIRE(s.kind == sexpresso::SexpValueKind::SEXP);
	REQUIRE(s.value.str.empty());
	REQUIRE(s.childCount() == 1);
	REQUIRE(s.childCount() == 1);
	REQUIRE(s.value.sexp[0].kind == sexpresso::SexpValueKind::SEXP);
	REQUIRE(s.value.sexp[0].value.sexp.empty());
	REQUIRE(s.toString() == "()");
}

TEST_CASE("Multiple empty sexp") {
	auto str = std::string{"()\n() ()"};

	auto err = std::string{};
	auto s = sexpresso::parse(str, err);

	REQUIRE(err.empty());
	REQUIRE(s.childCount() == 3);

	for(auto&& sc : s.value.sexp) {
		REQUIRE(sc.isNil());
	}

	REQUIRE(s.toString() == "() () ()");
}

TEST_CASE("Equality") {
	auto str = std::string{"hi there (what a cool (little list) parser) (library)"};

	auto err = std::string{};
	auto s = sexpresso::parse(str, err);

	REQUIRE(err.empty());

	auto outer = sexpresso::Sexp{};
	outer.addChild("hi");
	outer.addChild("there");


	auto what = sexpresso::Sexp{};
	what.addChild("what");
	what.addChild("a");
	what.addChild("cool");

	auto little = sexpresso::Sexp{};
	little.addChild("little");
	little.addChild("list");
	what.addChild(std::move(little));

	what.addChild("parser");

	outer.addChild(std::move(what));

	auto libholder = sexpresso::Sexp{};
	libholder.addChild("library");

	outer.addChild(std::move(libholder));

	REQUIRE(s.equal(outer));
	REQUIRE(str == s.toString());
}

TEST_CASE("Inequality") {
	auto err = std::string{};
	auto astr = std::string{"this (one is nothing)"};
	auto bstr = std::string{"like the (other)"};

	auto a = sexpresso::parse(astr, err);
	REQUIRE(err.empty());

	auto b = sexpresso::parse(bstr, err);
	REQUIRE(err.empty());

	REQUIRE(!a.equal(b));
}

TEST_CASE("String literal") {
	auto err = std::string{};
	auto s = sexpresso::parse("\"hello world\" hehe", err);
	REQUIRE(s.value.sexp[0].value.str == "hello world");
}

TEST_CASE("Hierarchy query") {
	auto err = std::string{};
	auto s = sexpresso::parse("(myshit (a (name me) (age 2)) (b (name you) (age 1)))", err);
	REQUIRE(s.getChildByPath("myshit/a/name")->equal(sexpresso::parse("name me", err)));
	REQUIRE(s.getChildByPath("myshit/a/age")->equal(sexpresso::parse("age 2", err)));
	REQUIRE(s.getChildByPath("myshit/a")->equal(sexpresso::parse("a (name me) (age 2)", err)));

	REQUIRE(s.getChildByPath("myshit/b/name")->equal(sexpresso::parse("name you", err)));
	REQUIRE(s.getChildByPath("myshit/b/age")->equal(sexpresso::parse("age 1", err)));
	REQUIRE(s.getChildByPath("myshit/b")->equal(sexpresso::parse("b (name you) (age 1)", err)));

	REQUIRE(s.getChildByPath("this/does/not/even/exist/dummy") == nullptr);
}

TEST_CASE("Unnacceptable syntax") {
	auto err = std::string{};

	sexpresso::parse("(((lol))", err);
	REQUIRE(!err.empty());

	sexpresso::parse("((rofl)))", err);
	REQUIRE(!err.empty());

	sexpresso::parse("(((\"i am gonna start a string but not close it))", err);
	REQUIRE(!err.empty());
}