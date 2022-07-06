#!/usr/bin/env python3
"""Module to interact with the i18n files easily."""
# Base imports
import os
import logging


class Key:
    """Class to represent a key in the i18n file."""

    def __init__(self, line: str, filename: str):
        """Initialize the key.

        Args:
            line (str): The line to parse.
            file (str): The original file name.
        """
        # Initialise the line
        self.line: str = line
        self.file: str = filename
        self.is_active: bool = True
        self.is_valid: bool = True
        # Set the key and value as empty to avoid errors
        self.key = self.line
        self.value = ''
        # Parse the line
        self.parse_line()
        # Get the language from the file name
        self.language = self.file.split('.')[-2]

    def parse_line(self):
        """Parse the line."""
        # Remove spaces around the = and around the line but not in the value
        self.remove_spaces()
        # Handle the comment
        if self.line.startswith('#'):
            self.is_active = False
            return
        # Handle empty lines
        if self.line.strip() == '':
            self.is_active = False
            self.is_valid = False
            return
        # Handle lines with only a key
        if '=' not in self.line:
            self.is_active = False
            self.is_valid = False
            return
        # Split the line
        line_split = self.line.split('=')
        # Handle line with =
        self.key: str = line_split[0].strip()
        # Add to the value all the other parts of the line
        self.value: str = '='.join(line_split[1:]).strip()
        # Remove the quotes
        self.value = self.value[1:-1]

    def remove_spaces(self):
        """Remove spaces around the = and the line."""
        # Remove spaces around the =
        self.line = self.line.replace(' = ', '=')
        self.line = self.line.replace('= ', '=')
        # Remove spaces around the line
        # Remove space at the beginning of the line
        self.line = self.line.lstrip()
        # Remove space at the end of the line
        self.line = self.line.rstrip()


class I18nData:
    """Class to interact with the i18n files easily."""

    def __init__(self, base_path=".", recursive=True, dry_run=False):
        """Initialise the class.

        Args:
            base_path (str, optional): The base path to search i18n files.
                                       Defaults to ".".
            recursive (bool, optional): Weather to search i18n files
                                        recursively. Defaults to True.
            dry_run (bool, optional): Weather to skip the writing of the files.
                                      Defaults to False.
        """
        # Log the initialisation
        logging.info("Initialising i18n data")
        # Initialise the class
        self.data = {}
        self.encoding = "utf-8"
        # Log the settings
        logging.info("Settings:")
        logging.info("  Base path: %s", base_path)
        logging.info("  Recursive: %s", recursive)
        logging.info("  Dry run: %s", dry_run)
        # Save the settings
        self.base_path = base_path
        self.recursive = recursive
        self.dry_run = dry_run
        # Load the i18n files
        self.load_i18n()

    def load_i18n(self):
        """Load the i18n files."""
        # Log the loading
        logging.info("Loading i18n files")
        # Clear the current data
        self.data = {}
        # Get the i18n files
        i18n_files = self.get_i18n_files()
        # Sort the i18n files
        self.data = self.sort_i18n_files(i18n_files)

    def get_i18n_files(self) -> list[str]:
        """Get the i18n files.

        Returns:
            list: The i18n files.
        """
        # Log the loading
        logging.info("Getting i18n files")
        # Get the i18n files
        i18n_files: list[str] = []
        # Get the i18n files
        for root, _, files in os.walk(self.base_path):
            # Get the i18n files in the current directory by filtering the
            # extension
            i18n_files.extend(
                os.path.join(root, file)
                for file in files if file.endswith(".i18n")
            )
        # Return the i18n files
        return i18n_files

    def sort_i18n_files(self, i18n_files: list) -> dict[str, list[Key]]:
        """Sort the i18n files.

        Args:
            i18n_files (list): The i18n files.

        Returns:
            dict: The i18n files sorted by language.
        """
        # Log the loading
        logging.info("Sorting i18n files")
        # Sort the i18n files
        i18n_files_language: dict[str, list[Key]] = {}
        # Sort the i18n files
        for i18n_file in i18n_files:
            # Get the language
            language = i18n_file.split(".")[-2]
            # Add the language to the list
            if language not in i18n_files_language:
                i18n_files_language[language] = []
            # Get the i18n data
            i18n_data = self.get_i18n_data(i18n_file)
            # Add the i18n data
            i18n_files_language[language].extend(i18n_data)
        # Return the i18n files
        return i18n_files_language

    def get_i18n_data(self, i18n_file: str) -> list[Key]:
        """Get the i18n data.

        Args:
            i18n_file (str): The i18n file.

        Returns:
            dict: The i18n data.
        """
        # Log the loading
        logging.debug("Getting i18n data from %s", i18n_file)
        # Get the i18n data
        i18n_data: list[Key] = []
        # Get the i18n data
        with open(i18n_file, "r", encoding=self.encoding) as i18n_file_handle:
            # Parse the lines
            for line in i18n_file_handle.read().splitlines():
                # Get the key
                key = Key(line, i18n_file)
                # Add the key
                i18n_data.append(key)
        # Return the i18n data
        return i18n_data

    def sort_i18n_data(self, original_order: str) -> None:
        """Sort the i18n data.

        Args:
            original_order (str): The original locale for the order of the
                                  i18n data.

        Sorting is done by copying the order of the original locale.
        """
        # Log the sorting
        logging.info("Sorting i18n data")
        # Extract the original order
        original_order_data = self.data[original_order]
        # Extract the keys names from the original order
        original_order_keys = [key.key for key in original_order_data]
        # Sort the keys
        for language, data in self.data.items():
            # Ignore the original order and the universal order
            if language in [original_order, "universal"]:
                continue
            # Sort the data
            data.sort(key=lambda key: original_order_keys.index(key.key))
            # Update the data
            self.data[language] = data

    def write_data(self) -> None:
        """Save the i18n data to the disk."""
        # Log the writing
        logging.info("Writing i18n data")
        # Iterate over the languages
        for language, data in self.data.items():
            # Generate locale's files
            files: dict[str, str] = self.generate_locale_files(language, data)
            # Write the files
            self.write_locale_files(files)

    def generate_locale_files(self, language: str, data: list[Key]) ->\
            dict[str, str]:
        """Generate the locale's files.

        Args:
            language (str): The language.
            data (list): The i18n data.

        Returns:
            list: The locale's files.
        """
        # Log the generation
        logging.debug("Generating locale's files for %s", language)
        # Generate the locale's files
        keys: dict[str, list[Key]] = {}
        # Get the keys by file, to handle bad sorting
        for key in data:
            # Get the file
            file = key.file
            # Add the file to the list
            if file not in keys:
                keys[file] = []
            # Add the key
            keys[file].append(key)
        # Return the files
        return {
            file:
            self.generate_file(language, file, keys)
                for file, keys in keys.items()
        }

    def generate_file(self, language: str, file: str, data: list[Key]) -> str:
        """Generate the locale's file.

        Args:
            language (str): The language.
            file (str): The file.
            data (list): The i18n data.

        Returns:
            str: The locale's file.
        """
        # Log the generation
        logging.debug("Generating locale's file for %s in %s", language, file)
        # Generate the locale's file
        file_data = ""
        # Iterate over the keys
        for key in data:
            # Check if the line is active
            if not key.is_active:
                file_data += f"{key.line}\n"
                continue
            # Else, generate the line
            file_data += f"{key.key} = \"{key.value}\"\n"
        # Return the file
        return file_data

    def write_locale_files(self, files: dict[str, str]) -> None:
        """Write the locale's files.

        Args:
            files (dict): The locale's files.
        """
        # Log the writing
        logging.info("Writing locale's files")
        # Iterate over the files
        for file, content in files.items():
            # Write the file
            self.write_file(file, content)

    def write_file(self, file: str, content: str) -> None:
        """Write the file.

        Args:
            file (str): The file.
            content (str): The content.
        """
        # Log the writing
        logging.debug("Writing file %s", file)
        # Write the file
        with open(file, "w", encoding=self.encoding) as file_handle:
            # Write the content if not in dry run
            if not self.dry_run:
                file_handle.write(content)


def main():
    """Run the tests."""
    # Setup the logging
    logging.basicConfig(level=logging.DEBUG)
    # Load the i18n files
    i18n = I18nData()
    # Print a part of the i18n data (the 10th key of each language)
    print("Part of the i18n data before sorting:")
    for language, keys in i18n.data.items():
        print("\t", language, keys[10].key, keys[10].value)
    # Sort the i18n data
    i18n.sort_i18n_data("fr")
    # Print a part of the i18n data (the 10th key of each language),
    # each key name should be the same
    last_key: str = ""
    first_key: bool = True
    print("Part of the i18n data after sorting:")
    for language, keys in i18n.data.items():
        print("\t", language, keys[10].key, keys[10].value)
        # Ignore universal
        if language == "universal":
            continue
        if not first_key:
            assert last_key == keys[10].key
        last_key = keys[10].key
        first_key = False
    # TODO: Assert that keys are sorted by files
    # Write the i18n data
    i18n.write_data()


if __name__ == "__main__":
    main()
