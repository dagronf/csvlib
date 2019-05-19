//
//  Encoding.hpp
//  csvlib
//
//  Created by Darren Ford on 27/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//

#pragma once

#ifdef ALLOW_ICU_EXTENSIONS

#include "unicode/ucsdet.h"
#include "unicode/uclean.h"

namespace csv {
namespace icu {
namespace encoding {

	struct detected_language {
		std::string name;
		std::string language;
		int32_t confidence;

		bool invalid() const { return name.empty(); }
	};

	detected_language TextEncodingForData(const char* data, size_t length) {
		UErrorCode uerr = U_ZERO_ERROR;
		UCharsetDetector *ucd = ucsdet_open ( &uerr );
		ucsdet_setText(ucd, data, static_cast<int>(length), &uerr);
		UCharsetMatch const * match = ucsdet_detect(ucd, &uerr);

		detected_language detected;
		detected.name = ucsdet_getName(match, &uerr);
		detected.language = ucsdet_getLanguage(match, &uerr);
		detected.confidence = ucsdet_getConfidence(match, &uerr);

		ucsdet_close(ucd);
		// u_cleanup(); // keep valgrind happy!!

		return detected;
	}

	detected_language TextEncodingForFile(const char* file) {
		UErrorCode u_glob_status = U_ZERO_ERROR;
		FILE* fp(0);

		u_init(&u_glob_status);
		if (U_SUCCESS(u_glob_status) &&
			(fp = fopen(file, "rb")) != NULL) {
			// Read (up to) the first 4k.  It should give us enough to detect the file
			char buf[4096];
			size_t read = fread(buf, 1, 4096, fp);

			fclose(fp);
			return TextEncodingForData(buf, read);
		}
		return detected_language();
	}

};
};
};

#endif
