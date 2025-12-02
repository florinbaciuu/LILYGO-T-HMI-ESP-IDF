# OTA Command

[![Component Registry](https://components.espressif.com/components/esp-qa/ota-cmd/badge.svg)](https://components.espressif.com/components/esp-qa/ota-cmd)

This repository contains `ota`/`simpleota` commands based esp-idf console.

There is no GitHub repository for this component. Please report issues to [iperf-cmd repository](https://github.com/espressif/iperf-cmd/issues) instead.

## Documentation

- Use `help` for overview of commands:

  ```
  ota  <action> [<url>] [--http-timeout=<int>]
    OTA upgrade command.
        <action>  start,
                  status,
                  end (reboot/esp_restart),
                  next (reboot to next app partition),
                  restore (reboot to first app partition).
           <url>  URL of server which hosts the firmware image.
    --http-timeout=<int>  http[s] response timeout (ms).

  simpleota  <action> [--no-reboot]
    simple OTA cmd, upgrade from local storage.
        <action>  start,status,next,restore,end.(same with ota cmd)
     --no-reboot  do not restart after 'simpleota next.'
  ```

### Installation

- To add a plugin command or any component from IDF component manager into your project, simply include an entry within the `idf_component.yml` file.

  ```yaml
  dependencies:
    esp-qa/ota-cmd:
      version: "^0.1.3"
  ```
- For more details refer [IDF Component Manager](https://docs.espressif.com/projects/idf-component-manager/en/latest/)
