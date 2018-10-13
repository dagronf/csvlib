//
//  TabulaRasaData.swift
//  Tabula Rasa
//
//  Created by Darren Ford on 13/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//

import Cocoa

class TabulaRasaData: NSObject {

	let url: URL

	enum FileType: NSInteger
	{
		case csv = 0
		case tsv = 1
	}

	let type: FileType
	private(set) var rawData: [[String]] = [[String]]()

	init(url: URL, type: FileType) {
		self.url = url
		self.type = type
		super.init()
	}

	func addRecords(record: [String])
	{
		self.rawData.append(record);
	}

	func load(completion: @escaping () -> Void)
	{
		rawData.removeAll()

		let sep: UnicodeScalar = self.type == .csv ? "," : "\t"
		let chopped = Int8(sep.value)

		DispatchQueue.global(qos: .userInitiated).async
		{ [weak self] in
			DSFCSVParser.parseFile((self?.url)!,
								   icuCodepage: nil,
								   separator: chopped,
								   fieldCallback: nil)
			{ (row: UInt, record: [String]) -> Bool in
					self?.addRecords(record: record)
					return true
			}

			DispatchQueue.main.async {
				completion()
			}

		}
	}

}
