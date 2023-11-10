#include <fstream>
#include <iostream>
#include "json/json.h"
#include "libdiagnostics++.h"

class BadJsonValue
{
  // TODO
};

class replayer
{
public:
  replayer (libdiagnostics::manager &mgr) : m_mgr (mgr) {}

  void replay_run (Json::Value run)
  {
    for (auto result : run["results"])
      replay_result (result);
  }

  void replay_result (Json::Value result)
  {
    enum diagnostic_level level (level_from_sarif (result["level"]));
    libdiagnostics::diagnostic diag (m_mgr.begin_diagnostic (level));

    if (0)
      {
	std::cout << "ruleId:" << result["ruleId"] << "\n";
	std::cout.flush ();
      }

    // TODO: codeFlows

    Json::Value loc = result["locations"][0]; // FIXME
    if (Json::Value physicalLocation = loc["physicalLocation"])
      diag.set_location (physical_location_from_sarif (physicalLocation));

    // FIXME:
    if (Json::Value logicalLocation0 = loc["logicalLocations"][0])
      diag.set_logical_location (logical_location_from_sarif (logicalLocation0));

    if (Json::Value taxa = result["taxa"])
      {
	if (0)
	  std::cout << "taxa:" << taxa << "\n";
	// FIXME: only if it exists, and is CWE
	diag.set_cwe (std::stoul (taxa[0]["id"].asString ()));
      }

    diag.finish ("%s", result["message"]["text"].asString ().c_str ());
  }

  static enum diagnostic_level level_from_sarif (Json::Value val)
  {
    if (val.asString () == "error")
      return DIAGNOSTIC_LEVEL_ERROR;;
    if (val.asString () == "warning")
      return DIAGNOSTIC_LEVEL_WARNING;
    if (val.asString () == "note") // FIXME
      return DIAGNOSTIC_LEVEL_NOTE;
    return DIAGNOSTIC_LEVEL_ERROR; // FIXME
  }

  libdiagnostics::physical_location
  physical_location_from_sarif (Json::Value val)
  {
    if (0)
      std::cout << "val:" << val << "\n";

    libdiagnostics::file f
      (m_mgr.new_file (val["artifactLocation"]["uri"].asCString (), nullptr));

    Json::Value region (val["region"]);
    libdiagnostics::line_num_t start_line = region["startLine"].asInt ();
    libdiagnostics::line_num_t end_line = start_line; // FIXME
    libdiagnostics::column_num_t start_column = region["startColumn"].asInt ();
    libdiagnostics::column_num_t end_column = region["endColumn"].asInt ();
    libdiagnostics::physical_location loc_start
      (m_mgr.new_location_from_file_line_column (f, start_line, start_column));
    libdiagnostics::physical_location loc_end
      (m_mgr.new_location_from_file_line_column (f, end_line, end_column));
    libdiagnostics::physical_location loc_range
      (m_mgr.new_location_from_range (loc_start,
				      loc_start,
				      loc_end));
    return loc_range;
  }

  libdiagnostics::logical_location
  logical_location_from_sarif (Json::Value val)
  {
    if (0)
      std::cout << "val:" << val << "\n";

    Json::Value name = val["name"];
    Json::Value fullyQualifiedName = val["fullyQualifiedName"];
    Json::Value decoratedName = val["decoratedName"];
    return m_mgr.new_logical_location
      (DIAGNOSTIC_LOGICAL_LOCATION_KIND_FUNCTION,
       libdiagnostics::logical_location (),
       name ? name.asCString () : nullptr,
       fullyQualifiedName ? fullyQualifiedName.asCString () : nullptr,
       decoratedName ? decoratedName.asCString () : nullptr);
  }

private:
  libdiagnostics::manager &m_mgr;
};

void replay_sarif (Json::Value root)
{
  libdiagnostics::manager mgr;
  mgr.add_text_sink (stderr, DIAGNOSTIC_COLORIZE_IF_TTY);

  replayer r (mgr);

  for (auto run : root["runs"])
    r.replay_run (run);
}

int main (int argc, char* argv[])
{
  std::ifstream ifs;
  ifs.open(argv[1]);

  Json::Value root;
  ifs >> root;
  replay_sarif (root);
  return EXIT_SUCCESS;
}
