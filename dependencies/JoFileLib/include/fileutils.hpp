#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Jo {
namespace Files {
namespace Utils {

	/// The files are ordered alphabetically in ascending order.
	class FileEnumerator
	{
		struct FileDesc {
			std::string name;
			uint64_t size;
		};
		std::vector<FileDesc>		m_files;
		std::vector<std::string>	m_directories;
	public:
		/// \brief Reads all filenames form the given directory.
		/// \details All the names are buffered so deletions or creations of
		///		files during iteration are not recognized.
		FileEnumerator( const std::string& _directory );

		/// \brief Refreshes the lists of names.
		/// \param [in] _directory Name of any directory to reload the list from.
		void Reset( const std::string& _directory );

		int GetNumFiles() const					{ return m_files.size(); }
		int GetNumDirectories() const			{ return m_directories.size(); }

		uint64_t GetFileSize( int _index ) const					{ return m_files[_index].size; }

		/// \brief Returns the file name without any path.
		///
		const std::string& GetFileName( int _index ) const			{ return m_files[_index].name; }

		const std::string& GetDirectoryName( int _index ) const		{ return m_directories[_index]; }

		/// \brief Returns the name of the directory whose content is enumerated.
		///
		const std::string& GetCurrentDirectoryName() const;
	};

	/// \brief Splits a given path name into directory and file.
	void SplitName( const std::string& _name, std::string& _dir, std::string& _file );

	/// \brief Returns the path of the file but without the
	///		file's name itself.
	/// \param [in] _name Arbitrary file name.
	std::string GetDirectory( const std::string& _name );

	/// \brief Create a directory. The name should not contain a file name.
	void MakeDir( const std::string& _name );

	/// \brief Tests if a file or directory exists.
	bool Exists( const std::string& _name );


	// Not implemented:
	/// \brief Takes an arbitrary file name makes sure that is has a full path.
	/// \param [in] _name Arbitrary file name.
	/*std::string MakeAbsolute( const std::string& _name );

	/// \brief Extracts the part after the last '.'.
	/// \param [in] _name Arbitrary file name.
	std::string GetExtension( const std::string& _name );

	/// \brief Returns the file name without the directory
	/// \param [in] _name Arbitrary file name.
	std::string GetFileName( const std::string& _name );*/
};
};
};