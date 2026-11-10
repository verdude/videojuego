#include "combat/commandrecognizer.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

using Edge = std::pair<Control, InputChange>;
constexpr auto pressed = InputChange::pressed;
constexpr auto released = InputChange::released;

// This adapter supplies input only. It does not recognize commands or implement
// timing. Each frame retains held state/history but replaces the current edges.
class ScriptedController : public InputController
{
public:
  void ingest(const std::vector<SDL_Event>&, uint64_t) override
  {
    throw std::logic_error("Tests supply logical input with frame()");
  }

  void reset() override { commands.clear(); }

  void frame(Tick tick, std::initializer_list<Edge> edges = {})
  {
    commands.clear();
    for (const auto& [control, change] : edges) {
      const Input input{ control, change, tick };
      commands.push_back(input);
      history.push_back(input);
      held.update(control, change == pressed, tick);
    }
  }
};

constexpr std::string_view one_config = R"([
  {"name": "one", "steps": [{"one": true}]}
])";

constexpr std::string_view motion_config = R"([
  {"name": "down_forward_one", "steps": [
    {"down": true, "forward": false},
    {"down": true, "forward": true},
    {"down": false, "forward": true, "one": true}
  ]}
])";

constexpr std::string_view charge_config = R"([
  {"name": "charge", "steps": [
    {"buttons": {"back": true}, "min_hold_ticks": 5}
  ]}
])";

class RecognizerTest : public testing::Test
{
protected:
  std::filesystem::path directory;
  std::filesystem::path config_path;
  std::unique_ptr<CommandRecognizer> recognizer;
  ScriptedController controller;

  void SetUp() override
  {
    const auto stamp =
      std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path();
    for (unsigned attempt = 0;; ++attempt) {
      directory = root / ("videojuego-recognizer-" + std::to_string(stamp) +
                          "-" + std::to_string(attempt));
      if (std::filesystem::create_directory(directory)) {
        break;
      }
    }
    config_path = directory / "commands.json";
  }

  void TearDown() override
  {
    std::error_code error;
    std::filesystem::remove_all(directory, error);
  }

  void write_config(std::string_view contents)
  {
    std::ofstream file(config_path);
    file << contents;
    file.close();
    if (!file) {
      throw std::runtime_error("Could not write test command config");
    }
  }

  void load(std::string_view contents)
  {
    write_config(contents);
    recognizer =
      std::make_unique<CommandRecognizer>(config_path.string().c_str());
    recognizer->load();
  }

  std::optional<CommandEvent> frame(Tick tick,
                                    std::initializer_list<Edge> edges = {},
                                    Facing facing = Facing::Right)
  {
    controller.frame(tick, edges);
    return recognizer->recognize(controller, facing, tick);
  }

  void expect_event(const std::optional<CommandEvent>& event,
                    Tick completed_at,
                    CommandId id = 0)
  {
    ASSERT_TRUE(event.has_value()) << "Expected a completed command";
    EXPECT_EQ(event->command, id);
    EXPECT_EQ(event->completedAt, completed_at);
  }
};

using CmdRecognizerChords = RecognizerTest;
using CmdRecognizerEvents = RecognizerTest;
using CmdRecognizerSequences = RecognizerTest;
using CmdRecognizerTiming = RecognizerTest;
using CmdRecognizerHolds = RecognizerTest;
using CmdRecognizerLoading = RecognizerTest;
using CmdRecognizerPlayers = RecognizerTest;

TEST_F(CmdRecognizerChords, NoInputDoesNotMatchRequiredButton)
{
  load(one_config);
  EXPECT_FALSE(frame(0));
}

TEST_F(CmdRecognizerChords, OnePressReportsCommandAndCompletionTick)
{
  load(one_config);
  expect_event(frame(12, { { Control::One, pressed } }), 12);
}

TEST_F(CmdRecognizerChords, UnspecifiedButtonsAreAllowed)
{
  load(one_config);
  expect_event(
    frame(2, { { Control::Right, pressed }, { Control::One, pressed } }), 2);
}

TEST_F(CmdRecognizerChords, ExplicitlyReleasedButtonRejectsExtraInput)
{
  load(R"([{"name":"one", "steps":[{"one":true, "two":false}]}])");
  EXPECT_FALSE(
    frame(2, { { Control::One, pressed }, { Control::Two, pressed } }));
}

TEST_F(CmdRecognizerChords, AllRequiredButtonsMustBeHeldTogether)
{
  load(R"([{"name":"one_two", "steps":[{"one":true, "two":true}]}])");
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
  expect_event(frame(2, { { Control::Two, pressed } }), 2);
}

TEST_F(CmdRecognizerChords, SeparateNonoverlappingPressesAreNotAChord)
{
  load(R"([{"name":"one_two", "steps":[{"one":true, "two":true}]}])");
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
  EXPECT_FALSE(frame(2, { { Control::One, released } }));
  EXPECT_FALSE(frame(3, { { Control::Two, pressed } }));
}

TEST_F(CmdRecognizerChords, ReturnsIndexOfMatchingDefinition)
{
  load(R"([
    {"name":"one", "steps":[{"one":true}]},
    {"name":"two", "steps":[{"two":true}]}
  ])");
  expect_event(frame(4, { { Control::Two, pressed } }), 4, 1);
}

class CmdRecognizerFacing
  : public RecognizerTest
  , public testing::WithParamInterface<Facing>
{};

TEST_P(CmdRecognizerFacing, ForwardFollowsFacing)
{
  load(R"([{"name":"forward", "steps":[{"forward":true, "back":false}]}])");
  const auto forward =
    GetParam() == Facing::Right ? Control::Right : Control::Left;
  expect_event(frame(1, { { forward, pressed } }, GetParam()), 1);
}

TEST_P(CmdRecognizerFacing, BackFollowsFacing)
{
  load(R"([{"name":"back", "steps":[{"back":true, "forward":false}]}])");
  const auto back =
    GetParam() == Facing::Right ? Control::Left : Control::Right;
  expect_event(frame(1, { { back, pressed } }, GetParam()), 1);
}

INSTANTIATE_TEST_SUITE_P(BothDirections,
                         CmdRecognizerFacing,
                         testing::Values(Facing::Left, Facing::Right),
                         [](const testing::TestParamInfo<Facing>& info) {
                           return info.param == Facing::Left ? "Left" : "Right";
                         });

TEST_F(CmdRecognizerEvents, HoldingDoesNotEmitEveryTick)
{
  load(one_config);
  expect_event(frame(10, { { Control::One, pressed } }), 10);
  EXPECT_FALSE(frame(11));
  EXPECT_FALSE(frame(12));
}

TEST_F(CmdRecognizerEvents, ReleaseAndRepressProducesAnotherOccurrence)
{
  load(one_config);
  expect_event(frame(10, { { Control::One, pressed } }), 10);
  EXPECT_FALSE(frame(11, { { Control::One, released } }));
  expect_event(frame(12, { { Control::One, pressed } }), 12);
}

TEST_F(CmdRecognizerEvents, UnrelatedEdgeDoesNotRetriggerHeldCommand)
{
  load(one_config);
  expect_event(frame(10, { { Control::One, pressed } }), 10);
  EXPECT_FALSE(frame(11, { { Control::Two, pressed } }));
}

TEST_F(CmdRecognizerEvents, ReleaseRequirementDoesNotFireFromInitialNeutral)
{
  load(R"([{"name":"release_one", "steps":[{"one":false}]}])");
  EXPECT_FALSE(frame(0));
}

TEST_F(CmdRecognizerEvents, ReleaseCommandFiresOnceOnRelease)
{
  load(R"([{"name":"release_one", "steps":[{"one":false}]}])");
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
  expect_event(frame(2, { { Control::One, released } }), 2);
  EXPECT_FALSE(frame(3));
}

TEST_F(CmdRecognizerSequences, OrderedMotionCompletesOnFinalStep)
{
  load(motion_config);
  EXPECT_FALSE(frame(10, { { Control::Down, pressed } }));
  EXPECT_FALSE(frame(11, { { Control::Right, pressed } }));
  expect_event(
    frame(12, { { Control::Down, released }, { Control::One, pressed } }), 12);
}

TEST_F(CmdRecognizerSequences, FinalChordAloneCannotSatisfySequence)
{
  load(motion_config);
  EXPECT_FALSE(
    frame(1, { { Control::Right, pressed }, { Control::One, pressed } }));
}

TEST_F(CmdRecognizerSequences, StepsInReverseOrderDoNotComplete)
{
  load(R"([{"name":"one_then_two", "steps":[
    {"one":true, "two":false}, {"two":true, "one":false}
  ]}])");
  EXPECT_FALSE(frame(1, { { Control::Two, pressed } }));
  EXPECT_FALSE(
    frame(2, { { Control::Two, released }, { Control::One, pressed } }));
}

TEST_F(CmdRecognizerSequences, OneOccurrenceCannotSatisfyTwoIdenticalSteps)
{
  load(R"([{"name":"double_one", "steps":[{"one":true}, {"one":true}]}])");
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
  EXPECT_FALSE(frame(2));
}

TEST_F(CmdRecognizerSequences, TwoDistinctPressesCompleteIdenticalSteps)
{
  load(R"([{"name":"double_one", "steps":[{"one":true}, {"one":true}]}])");
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
  EXPECT_FALSE(frame(2, { { Control::One, released } }));
  expect_event(frame(3, { { Control::One, pressed } }), 3);
}

TEST_F(CmdRecognizerSequences, DoubleTapIncludesReleaseBetweenPresses)
{
  load(R"([{"name":"dash", "steps":[
    {"forward":true}, {"forward":false}, {"forward":true}
  ]}])");
  EXPECT_FALSE(frame(1, { { Control::Right, pressed } }));
  EXPECT_FALSE(frame(2, { { Control::Right, released } }));
  expect_event(frame(3, { { Control::Right, pressed } }), 3);
}

TEST_F(CmdRecognizerSequences, CompletedSequenceDoesNotRepeatWhileHeld)
{
  load(motion_config);
  frame(1, { { Control::Down, pressed } });
  frame(2, { { Control::Right, pressed } });
  expect_event(
    frame(3, { { Control::Down, released }, { Control::One, pressed } }), 3);
  EXPECT_FALSE(frame(4));
}

TEST_F(CmdRecognizerTiming, GapAllowsCompletionAtInclusiveBoundary)
{
  load(R"([{"name":"one_two", "max_gap_ticks":4, "steps":[
    {"one":true, "two":false}, {"one":false, "two":true}
  ]}])");
  EXPECT_FALSE(frame(10, { { Control::One, pressed } }));
  expect_event(
    frame(14, { { Control::One, released }, { Control::Two, pressed } }), 14);
}

TEST_F(CmdRecognizerTiming, GapRejectsOneTickPastBoundary)
{
  load(R"([{"name":"one_two", "max_gap_ticks":4, "steps":[
    {"one":true, "two":false}, {"one":false, "two":true}
  ]}])");
  EXPECT_FALSE(frame(10, { { Control::One, pressed } }));
  EXPECT_FALSE(
    frame(15, { { Control::One, released }, { Control::Two, pressed } }));
}

TEST_F(CmdRecognizerTiming, GapUsesTickDifferenceNotNumberOfCalls)
{
  load(R"([{"name":"one_two", "max_gap_ticks":4, "steps":[
    {"one":true, "two":false}, {"one":false, "two":true}
  ]}])");
  frame(10, { { Control::One, pressed } });
  EXPECT_FALSE(
    frame(1000, { { Control::One, released }, { Control::Two, pressed } }));
}

TEST_F(CmdRecognizerTiming, TotalWindowAllowsItsInclusiveBoundary)
{
  auto config = nlohmann::json::parse(motion_config);
  config[0]["max_gap_ticks"] = 4;
  config[0]["max_total_ticks"] = 8;
  load(config.dump());
  frame(10, { { Control::Down, pressed } });
  frame(14, { { Control::Right, pressed } });
  expect_event(
    frame(18, { { Control::Down, released }, { Control::One, pressed } }), 18);
}

TEST_F(CmdRecognizerTiming, TotalWindowCanExpireEvenWhenEachGapIsValid)
{
  auto config = nlohmann::json::parse(motion_config);
  config[0]["max_gap_ticks"] = 4;
  config[0]["max_total_ticks"] = 7;
  load(config.dump());
  frame(10, { { Control::Down, pressed } });
  frame(14, { { Control::Right, pressed } });
  EXPECT_FALSE(
    frame(18, { { Control::Down, released }, { Control::One, pressed } }));
}

TEST_F(CmdRecognizerTiming, FreshAttemptCanSucceedAfterTimeout)
{
  load(R"([{"name":"one_two", "max_gap_ticks":4, "steps":[
    {"one":true, "two":false}, {"one":false, "two":true}
  ]}])");
  frame(1, { { Control::One, pressed } });
  EXPECT_FALSE(
    frame(6, { { Control::One, released }, { Control::Two, pressed } }));
  frame(7, { { Control::Two, released } });
  EXPECT_FALSE(frame(8, { { Control::One, pressed } }));
  expect_event(
    frame(10, { { Control::One, released }, { Control::Two, pressed } }), 10);
}

TEST_F(CmdRecognizerHolds, HoldWaitsForThresholdThenEmitsOnce)
{
  load(charge_config);
  EXPECT_FALSE(frame(10, { { Control::Left, pressed } }));
  EXPECT_FALSE(frame(14));
  expect_event(frame(15), 15);
  EXPECT_FALSE(frame(16));
}

TEST_F(CmdRecognizerHolds, ReleaseBeforeThresholdDoesNotMatch)
{
  load(charge_config);
  EXPECT_FALSE(frame(10, { { Control::Left, pressed } }));
  EXPECT_FALSE(frame(14, { { Control::Left, released } }));
  EXPECT_FALSE(frame(15));
}

TEST_F(CmdRecognizerHolds, ReleaseAndRepressRestartsHoldDuration)
{
  load(charge_config);
  frame(10, { { Control::Left, pressed } });
  frame(14, { { Control::Left, released } });
  EXPECT_FALSE(frame(15, { { Control::Left, pressed } }));
  EXPECT_FALSE(frame(19));
  expect_event(frame(20), 20);
}

TEST_F(CmdRecognizerHolds, EveryRequiredButtonMustMeetHoldDuration)
{
  load(R"([{"name":"hold_both", "steps":[
    {"buttons":{"one":true, "two":true}, "min_hold_ticks":5}
  ]}])");
  EXPECT_FALSE(frame(10, { { Control::One, pressed } }));
  EXPECT_FALSE(frame(13, { { Control::Two, pressed } }));
  EXPECT_FALSE(frame(15));
  expect_event(frame(18), 18);
}

TEST_F(CmdRecognizerHolds, ChargedStepCanPrecedeAnotherStep)
{
  load(R"([{"name":"charge_forward_one", "max_gap_ticks":4, "steps":[
    {"buttons":{"back":true}, "min_hold_ticks":5},
    {"back":false, "forward":true, "one":true}
  ]}])");
  EXPECT_FALSE(frame(10, { { Control::Left, pressed } }));
  EXPECT_FALSE(frame(15));
  expect_event(frame(17,
                     { { Control::Left, released },
                       { Control::Right, pressed },
                       { Control::One, pressed } }),
               17);
}

TEST_F(CmdRecognizerPlayers, HoldingOnOneControllerDoesNotSuppressAnother)
{
  load(one_config);
  ScriptedController second;
  expect_event(frame(1, { { Control::One, pressed } }), 1);
  second.frame(1, { { Control::One, pressed } });
  expect_event(recognizer->recognize(second, Facing::Right, 1), 1);
  EXPECT_FALSE(frame(2));
  second.frame(2);
  EXPECT_FALSE(recognizer->recognize(second, Facing::Right, 2));
}

TEST_F(CmdRecognizerPlayers, PartialSequenceCannotBeSharedBetweenControllers)
{
  load(motion_config);
  ScriptedController second;
  frame(1, { { Control::Down, pressed } });
  frame(2, { { Control::Right, pressed } });
  second.frame(3, { { Control::Right, pressed }, { Control::One, pressed } });
  EXPECT_FALSE(recognizer->recognize(second, Facing::Right, 3));
  expect_event(
    frame(3, { { Control::Down, released }, { Control::One, pressed } }), 3);
}

TEST_F(CmdRecognizerLoading, EmptyCommandListMatchesNothing)
{
  load("[]");
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
}

TEST_F(CmdRecognizerLoading, MissingFileReportsAnError)
{
  CommandRecognizer missing(config_path.string().c_str());
  EXPECT_THROW(missing.load(), std::exception);
}

TEST_F(CmdRecognizerLoading, MalformedJsonReportsAnError)
{
  EXPECT_THROW(load("[broken json"), std::exception);
}

TEST_F(CmdRecognizerLoading, UnknownControlReportsAnError)
{
  EXPECT_THROW(load(R"([{"name":"typo", "steps":[{"oen":true}]}])"),
               std::exception);
}

TEST_F(CmdRecognizerLoading, EmptyStepSequenceIsRejected)
{
  EXPECT_THROW(load(R"([{"name":"empty", "steps":[]}])"), std::exception);
}

TEST_F(CmdRecognizerLoading, ReloadReplacesPreviousDefinitions)
{
  load(one_config);
  write_config(R"([{"name":"two", "steps":[{"two":true}]}])");
  recognizer->load();
  EXPECT_FALSE(frame(1, { { Control::One, pressed } }));
  expect_event(
    frame(2, { { Control::One, released }, { Control::Two, pressed } }), 2, 0);
}

} // namespace
